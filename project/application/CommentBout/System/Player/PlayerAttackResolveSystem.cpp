#include "stdafx.h"
#include "PlayerAttackResolveSystem.h"

#include "application/CommentBout/Component/AttackDamageComponent.h"
#include "application/CommentBout/Component/DamageRequestComponent.h"
#include "application/CommentBout/Component/Enemy/EnemyComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Utility/CBCollisionMask.h"
#include "application/CommentBout/Collision/Component/Collider2DComponent.h"
#include "application/CommentBout/Collision/Component/Collider3DComponent.h"
#include "application/CommentBout/Collision/Component/ProjectedColliderComponent.h"
#include "application/CommentBout/Collision/Utility/CollisionAlgorithms.h"
#include "engine/Functions/ECS/Component/CameraComponent.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace {
	// プレイヤーの攻撃が当たる可能性のある敵を列挙するためのクエリ
bool ContainsEntity(const std::vector<No::Entity>& entities, No::Entity entity) {
	return std::find(entities.begin(), entities.end(), entity) != entities.end();
}

// ダメージリクエストを発行するユーティリティ関数
void EmitDamageRequest(
	No::Registry& registry,
	No::Entity target,
	No::Entity source,
	int amount,
	bool ignoreInvincible
) {
	if (target == No::nullEntity || amount <= 0) {
		return;
	}

	auto requestEntity = registry.GenerateEntity();
	auto* request = registry.AddComponent<DamageRequestComponent>(requestEntity);
	if (!request) {
		registry.DestroyEntity(requestEntity);
		return;
	}
	request->target = target;
	request->source = source;
	request->amount = amount;
	request->ignoreInvincible = ignoreInvincible;
}

// AABB 内に点があるか判定するユーティリティ関数
bool IsPointInAABB(
	const No::Vector2& point,
	const No::Vector2& rectCenter,
	const No::Vector2& rectSize
) {
	const No::Vector2 half = rectSize * 0.5f;
	return point.x >= rectCenter.x - half.x &&
		point.x <= rectCenter.x + half.x &&
		point.y >= rectCenter.y - half.y &&
		point.y <= rectCenter.y + half.y;
}

// 凸多角形の内部に点があるか判定するユーティリティ関数
bool IsPointInConvexHull(const std::vector<No::Vector2>& hull, const No::Vector2& point) {
	const size_t n = hull.size();
	if (n < 3) {
		return false;
	}

	float sign = 0.0f;
	for (size_t i = 0; i < n; ++i) {
		const No::Vector2& a = hull[i];
		const No::Vector2& b = hull[(i + 1) % n];
		const No::Vector2 ab = b - a;
		const No::Vector2 ap = point - a;
		const float cross = ab.x * ap.y - ab.y * ap.x;
		if (std::abs(cross) <= 0.0001f) {
			continue;
		}
		if (sign == 0.0f) {
			sign = cross;
		} else if ((sign > 0.0f && cross < 0.0f) || (sign < 0.0f && cross > 0.0f)) {
			return false;
		}
	}
	return true;
}
// 投影されたコライダーに点が含まれているか判定するユーティリティ関数
bool IsPointInsideProjected(
	const CommentBoutCollision::ProjectedColliderComponent& projected,
	const No::Vector2& point
) {
	if (!projected.isVisible) {
		return false;
	}
	if (projected.isBox) {
		return IsPointInConvexHull(projected.convexHull, point);
	}
	const No::Vector2 d = point - projected.screenPosition;
	return d.LengthSquared() <= projected.screenRadius * projected.screenRadius;
}
// 投影されたコライダーと攻撃のAABBが重なっているか判定するユーティリティ関数
bool CheckProjectedVsAttack(
	const CommentBoutCollision::ProjectedColliderComponent& projected,
	const CommentBoutCollision::Collider2DComponent& attackCollider
) {
	if (!projected.isVisible) {
		return false;
	}
	if (projected.isBox) {
		return CommentBoutCollision::CollisionAlgorithms::CheckConvexHullAABB(
			projected.convexHull,
			attackCollider.screenPosition,
			attackCollider.worldSize
		);
	}
	return CommentBoutCollision::CollisionAlgorithms::CheckCircleAABB(
		projected.screenPosition,
		projected.screenRadius,
		attackCollider.screenPosition,
		attackCollider.worldSize
	);
}
// 投影されたコライダーの衝突判定に使用するサンプル点を収集するユーティリティ関数
void GatherProjectedSamplePoints(
	const CommentBoutCollision::ProjectedColliderComponent& projected,
	std::vector<No::Vector2>& outPoints
) {
	outPoints.clear();
	outPoints.push_back(projected.screenPosition);

	if (projected.isBox) {
		const size_t n = projected.convexHull.size();
		for (size_t i = 0; i < n; ++i) {
			const No::Vector2& a = projected.convexHull[i];
			const No::Vector2& b = projected.convexHull[(i + 1) % n];
			outPoints.push_back(a);
			outPoints.push_back((a + b) * 0.5f);
		}
		return;
	}

	const float r = std::max(1.0f, projected.screenRadius);
	outPoints.push_back(projected.screenPosition + No::Vector2(r, 0.0f));
	outPoints.push_back(projected.screenPosition + No::Vector2(-r, 0.0f));
	outPoints.push_back(projected.screenPosition + No::Vector2(0.0f, r));
	outPoints.push_back(projected.screenPosition + No::Vector2(0.0f, -r));
}
// ワールド座標の点がカメラからどれだけ離れているか（カメラ空間でのZ値）を計算するユーティリティ関数
float ComputeCameraDepth(
	const No::Vector3& worldPos,
	const No::CameraComponent* camera
) {
	if (!camera) {
		return worldPos.z;
	}

	const No::Matrix4x4& vp = camera->forGPU.viewProjection;
	const float clipZ =
		worldPos.x * vp.m[0][2] +
		worldPos.y * vp.m[1][2] +
		worldPos.z * vp.m[2][2] +
		vp.m[3][2];
	const float clipW =
		worldPos.x * vp.m[0][3] +
		worldPos.y * vp.m[1][3] +
		worldPos.z * vp.m[2][3] +
		vp.m[3][3];

	if (std::abs(clipW) <= 0.000001f) {
		return clipZ;
	}
	return clipZ / clipW;
}
}

void PlayerAttackResolveSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);

	No::CameraComponent* activeCamera = nullptr;
	auto activeCameraView = registry.View<No::CameraComponent, No::ActiveCameraTag>();
	auto activeCameraIt = activeCameraView.begin();
	if (activeCameraIt != activeCameraView.end()) {
		activeCamera = registry.GetComponent<No::CameraComponent>(*activeCameraIt);
	}

	struct EnemyAttackTarget {
		No::Entity entity;
		EnemyComponent* enemy;
		HealthComponent* health;
		CommentBoutCollision::Collider3DComponent* collider3D;
		CommentBoutCollision::ProjectedColliderComponent* projected;
		float depth;
	};
	struct Occluder {
		No::Entity entity;
		const CommentBoutCollision::ProjectedColliderComponent* projected;
		float depth;
	};

	std::vector<EnemyAttackTarget> attackTargets;
	auto enemyAttackView = registry.View<CBRailEnemyTag, EnemyComponent, CommentBoutCollision::Collider3DComponent, CommentBoutCollision::ProjectedColliderComponent>();
	for (auto entity : enemyAttackView) {
		auto* enemy = registry.GetComponent<EnemyComponent>(entity);
		auto* health = registry.GetComponent<HealthComponent>(entity);
		auto* collider3D = registry.GetComponent<CommentBoutCollision::Collider3DComponent>(entity);
		auto* projected = registry.GetComponent<CommentBoutCollision::ProjectedColliderComponent>(entity);
		if (!enemy || !health || !collider3D || !projected || !projected->isVisible || health->isDead || health->hp <= 0) {
			continue;
		}

		EnemyAttackTarget target{};
		target.entity = entity;
		target.enemy = enemy;
		target.health = health;
		target.collider3D = collider3D;
		target.projected = projected;
		target.depth = ComputeCameraDepth(collider3D->worldPosition, activeCamera);
		attackTargets.push_back(target);
	}

	std::vector<Occluder> occluders;
	auto projectedView = registry.View<CommentBoutCollision::ProjectedColliderComponent, CommentBoutCollision::Collider3DComponent>();
	for (auto entity : projectedView) {
		auto* projected = registry.GetComponent<CommentBoutCollision::ProjectedColliderComponent>(entity);
		auto* collider3D = registry.GetComponent<CommentBoutCollision::Collider3DComponent>(entity);
		if (!projected || !collider3D || !projected->isVisible) {
			continue;
		}

		const bool enemyOccluder = (projected->collisionLayer == CommentBout::CollisionLayer::CBEnemy);
		const bool fieldOccluder = registry.Has<CBFieldOccluderTag>(entity);
		if (!enemyOccluder && !fieldOccluder) {
			continue;
		}

		// 第1段階仕様:
		//  - プレイヤー攻撃のみ、敵 + Buildingオブジェクトで遮蔽する
		//  - 敵弾遮蔽は将来拡張で対応する
		Occluder occluder{};
		occluder.entity = entity;
		occluder.projected = projected;
		occluder.depth = ComputeCameraDepth(collider3D->worldPosition, activeCamera);
		occluders.push_back(occluder);
	}

	std::vector<No::Vector2> samplePoints;
	auto attackView = registry.View<CommentBoutCollision::Collider2DComponent, AttackDamageComponent>();
	for (auto attackEntity : attackView) {
		auto* attackCollider = registry.GetComponent<CommentBoutCollision::Collider2DComponent>(attackEntity);
		auto* attackDamage = registry.GetComponent<AttackDamageComponent>(attackEntity);
		if (!attackCollider || !attackDamage) {
			continue;
		}
		if (attackCollider->collisionLayer != CommentBout::CollisionLayer::CBPlayerAttack) {
			continue;
		}

		// 攻撃判定は寿命中ずっと行う。
		// 以前の「初回フレームのみ」判定は更新順の影響で取りこぼしを生みやすかったため廃止する。
		// 同一敵への多重ヒットは hitEnemies で抑止し、敵同士遮蔽は従来通り維持する。
		for (auto& target : attackTargets) {
			if (!target.enemy || !target.health || target.health->isDead || target.health->hp <= 0) {
				continue;
			}
			if (ContainsEntity(attackDamage->hitEnemies, target.entity)) {
				continue;
			}

			if (!CheckProjectedVsAttack(*target.projected, *attackCollider)) {
				continue;
			}

			GatherProjectedSamplePoints(*target.projected, samplePoints);
			bool canHitVisiblePart = false;

			for (const auto& p : samplePoints) {
				if (!IsPointInAABB(p, attackCollider->screenPosition, attackCollider->worldSize)) {
					continue;
				}
				if (!IsPointInsideProjected(*target.projected, p)) {
					continue;
				}

				bool occluded = false;
				for (const auto& occluder : occluders) {
					if (occluder.entity == target.entity) {
						continue;
					}
					if (occluder.depth >= target.depth - 0.0001f) {
						continue;
					}
					if (IsPointInsideProjected(*occluder.projected, p)) {
						occluded = true;
						break;
					}
				}

				if (!occluded) {
					canHitVisiblePart = true;
					break;
				}
			}

			if (!canHitVisiblePart) {
				continue;
			}

			const int damage = std::max(1, attackDamage->damage);
			EmitDamageRequest(registry, target.entity, attackEntity, damage, false);
			attackDamage->hitEnemies.push_back(target.entity);
		}
	}

	// 自機攻撃側は遮蔽判定を優先し、イベント多経路を排除して命中条件の追跡性を高めている。
}
