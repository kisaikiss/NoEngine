#include "stdafx.h"
#include "EnemySystem.h"

#include "application/CommentBout/Component/RailCameraComponent.h"
#include "application/CommentBout/Component/EnemyComponent.h"
#include "application/CommentBout/Component/AttackDamageComponent.h"
#include "application/CommentBout/Component/PlayerHealthComponent.h"
#include "application/CommentBout/Component/PlayerHitboxComponent.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Utility/CBCollisionMask.h"
#include "application/CommentBout/Collision/Component/Collider2DComponent.h"
#include "application/CommentBout/Collision/Component/Collider3DComponent.h"
#include "application/CommentBout/Collision/Component/ProjectedColliderComponent.h"
#include "application/CommentBout/Collision/Utility/CollisionAlgorithms.h"
#include "engine/Functions/ECS/Component/MaterialComponent.h"
#include "engine/Functions/Renderer/Primitive.h"
#include <algorithm>
#include <cmath>

namespace No {
using ::NoEngine::Primitive;
}

namespace {
No::Vector3 NormalizeOrDefault(const No::Vector3& v, const No::Vector3& fallback) {
	if (v.LengthSquared() <= 0.000001f) {
		return fallback;
	}
	return v.Normalize();
}

bool CheckSphereVsObb(const CommentBoutCollision::Collider3DComponent& sphereCollider, const PlayerHitboxComponent& hitbox) {
	No::Vector3 d = sphereCollider.worldPosition - hitbox.obbCenter;
	No::Vector3 closest = hitbox.obbCenter;

	const No::Vector3 axes[3] = { hitbox.obbAxisX, hitbox.obbAxisY, hitbox.obbAxisZ };
	const float extents[3] = { hitbox.obbHalfExtents.x, hitbox.obbHalfExtents.y, hitbox.obbHalfExtents.z };

	for (int i = 0; i < 3; ++i) {
		float dist = d.Dot(axes[i]);
		dist = std::max(-extents[i], std::min(extents[i], dist));
		closest += axes[i] * dist;
	}

	const float r = std::max(0.0f, sphereCollider.worldRadius);
	return (sphereCollider.worldPosition - closest).LengthSquared() <= (r * r);
}

bool CheckAabbVsObb(const CommentBoutCollision::Collider3DComponent& aabb, const PlayerHitboxComponent& hitbox) {
	const float epsilon = 1e-5f;
	const No::Vector3 aabbCenter = aabb.worldPosition;
	const No::Vector3 aabbExtent = aabb.worldBoxSize * 0.5f;

	const No::Vector3 U[3] = { hitbox.obbAxisX, hitbox.obbAxisY, hitbox.obbAxisZ };
	const No::Vector3 A[3] = { No::Vector3::RIGHT, No::Vector3::UP, No::Vector3::FORWARD };
	const float E[3] = { hitbox.obbHalfExtents.x, hitbox.obbHalfExtents.y, hitbox.obbHalfExtents.z };
	const float B[3] = { aabbExtent.x, aabbExtent.y, aabbExtent.z };

	float R[3][3] = {};
	float AbsR[3][3] = {};
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			R[i][j] = U[i].Dot(A[j]);
			AbsR[i][j] = std::abs(R[i][j]) + epsilon;
		}
	}

	const No::Vector3 tWorld = aabbCenter - hitbox.obbCenter;
	const float t[3] = { tWorld.Dot(U[0]), tWorld.Dot(U[1]), tWorld.Dot(U[2]) };

	for (int i = 0; i < 3; ++i) {
		const float ra = E[i];
		const float rb = B[0] * AbsR[i][0] + B[1] * AbsR[i][1] + B[2] * AbsR[i][2];
		if (std::abs(t[i]) > ra + rb) {
			return false;
		}
	}

	for (int j = 0; j < 3; ++j) {
		const float ra = E[0] * AbsR[0][j] + E[1] * AbsR[1][j] + E[2] * AbsR[2][j];
		const float rb = B[j];
		const float val = std::abs((j == 0) ? tWorld.x : (j == 1 ? tWorld.y : tWorld.z));
		if (val > ra + rb) {
			return false;
		}
	}

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			const int i1 = (i + 1) % 3;
			const int i2 = (i + 2) % 3;
			const int j1 = (j + 1) % 3;
			const int j2 = (j + 2) % 3;

			const float ra = E[i1] * AbsR[i2][j] + E[i2] * AbsR[i1][j];
			const float rb = B[j1] * AbsR[i][j2] + B[j2] * AbsR[i][j1];
			const float val = std::abs(t[i2] * R[i1][j] - t[i1] * R[i2][j]);
			if (val > ra + rb) {
				return false;
			}
		}
	}

	return true;
}

bool CheckEnemyVsPlayerHitbox(const CommentBoutCollision::Collider3DComponent& enemyCollider, const CommentBoutCollision::Collider3DComponent& playerCollider, const PlayerHitboxComponent& hitbox) {
	if (hitbox.useObbCollision && hitbox.obbValid) {
		if (enemyCollider.shapeType == CommentBoutCollision::ShapeType3D::Sphere) {
			return CheckSphereVsObb(enemyCollider, hitbox);
		}
		return CheckAabbVsObb(enemyCollider, hitbox);
	}

	if (enemyCollider.shapeType == CommentBoutCollision::ShapeType3D::Box && playerCollider.shapeType == CommentBoutCollision::ShapeType3D::Box) {
		return CommentBoutCollision::CollisionAlgorithms::CheckAABB3DAABB3D(
			enemyCollider.worldPosition,
			enemyCollider.worldBoxSize,
			playerCollider.worldPosition,
			playerCollider.worldBoxSize
		);
	}
	if (enemyCollider.shapeType == CommentBoutCollision::ShapeType3D::Sphere && playerCollider.shapeType == CommentBoutCollision::ShapeType3D::Sphere) {
		return CommentBoutCollision::CollisionAlgorithms::CheckSphereSphere(
			enemyCollider.worldPosition,
			enemyCollider.worldRadius,
			playerCollider.worldPosition,
			playerCollider.worldRadius
		);
	}
	if (enemyCollider.shapeType == CommentBoutCollision::ShapeType3D::Sphere && playerCollider.shapeType == CommentBoutCollision::ShapeType3D::Box) {
		return CommentBoutCollision::CollisionAlgorithms::CheckSphereAABB3D(
			enemyCollider.worldPosition,
			enemyCollider.worldRadius,
			playerCollider.worldPosition,
			playerCollider.worldBoxSize
		);
	}
	return CommentBoutCollision::CollisionAlgorithms::CheckSphereAABB3D(
		playerCollider.worldPosition,
		playerCollider.worldRadius,
		enemyCollider.worldPosition,
		enemyCollider.worldBoxSize
	);
}

void DrawColliderDebug(const CommentBoutCollision::Collider3DComponent& collider, const NoEngine::Math::Color& color) {
	if (collider.shapeType == CommentBoutCollision::ShapeType3D::Box) {
		No::Primitive::DrawCube(collider.worldPosition, collider.worldBoxSize, color);
	} else {
		No::Primitive::DrawSphere(collider.worldPosition, collider.worldRadius, color, 10, 10);
	}
}

void SpawnRailEnemiesInternal(No::Registry& registry, const RailEnemySpawnEventParams& params) {
	const No::Vector3 direction = NormalizeOrDefault(params.moveDirection, No::Vector3(0.0f, 0.0f, -1.0f));
	const int spawnCount = std::max(1, params.count);

	for (int i = 0; i < spawnCount; ++i) {
		auto enemyEntity = registry.GenerateEntity();
		registry.AddComponent<CBRailEnemyTag>(enemyEntity);

		auto* transform = registry.AddComponent<No::TransformComponent>(enemyEntity);
		transform->translate = params.spawnPosition + direction * (params.spawnSpacing * static_cast<float>(i));
		transform->scale = { 0.7f, 0.7f, 0.7f };

		auto* mesh = registry.AddComponent<No::MeshComponent>(enemyEntity);
		auto* material = registry.AddComponent<No::MaterialComponent>(enemyEntity);
		No::ModelLoader::LoadModel("commentbout_rail_enemy_cube", "resources/game/td_3105/Model/cube/cube.obj", mesh);
		material->materials = No::ModelLoader::GetMaterial("commentbout_rail_enemy_cube");
		material->color = { 1.0f, 1.0f, 1.0f, 1.0f };
		material->psoName = L"Renderer : Default PSO";
		material->psoId = NoEngine::Render::GetPSOID(material->psoName);
		material->rootSigId = NoEngine::Render::GetRootSignatureID(material->psoName);

		auto* enemy = registry.AddComponent<EnemyComponent>(enemyEntity);
		enemy->maxHp = std::max(1, params.hp);
		enemy->hp = enemy->maxHp;
		enemy->moveSpeed = std::max(0.0f, params.moveSpeed);
		enemy->moveDirection = direction;
		enemy->groupId = params.spawnGroupId;

		auto* collider3D = registry.AddComponent<CommentBoutCollision::Collider3DComponent>(enemyEntity);
		collider3D->shapeType = CommentBoutCollision::ShapeType3D::Box;
		collider3D->useScaleAsBox = true;
		collider3D->boxSizeMultiplier = { 1.0f, 1.0f, 1.0f };
		collider3D->collisionLayer = CommentBout::CollisionLayer::CBEnemy;
		collider3D->collisionMask = CommentBout::CollisionLayer::CBPlayerAttack;

		auto* projected = registry.AddComponent<CommentBoutCollision::ProjectedColliderComponent>(enemyEntity);
		projected->source3DEntity = enemyEntity;
	}
}
}

void SpawnRailEnemies(No::Registry& registry, const RailEnemySpawnEventParams& params) {
	SpawnRailEnemiesInternal(registry, params);
}

void EnemySystem::Update(No::Registry& registry, float deltaTime) {
	static bool drawColliderDebug = true;
#ifdef USE_IMGUI
	ImGui::Begin("Enemy Debug");
	ImGui::Checkbox("Draw Enemy Colliders", &drawColliderDebug);
	ImGui::Separator();
#endif

	auto view = registry.View<CBRailEnemyTag, EnemyComponent, No::TransformComponent, No::MaterialComponent, CommentBoutCollision::Collider3DComponent>();
	for (auto entity : view) {
		auto* enemy = registry.GetComponent<EnemyComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* material = registry.GetComponent<No::MaterialComponent>(entity);
		auto* collider3D = registry.GetComponent<CommentBoutCollision::Collider3DComponent>(entity);
		if (!enemy || !transform || !material || !collider3D) {
			continue;
		}

		const No::Vector3 moveDir = NormalizeOrDefault(enemy->moveDirection, No::Vector3(0.0f, 0.0f, -1.0f));
		transform->translate += moveDir * enemy->moveSpeed * deltaTime;

		int attackDamage = 1;
		bool isHitByPlayerAttack = false;
		if (collider3D->isColliding) {
			auto* hit2D = registry.GetComponent<CommentBoutCollision::Collider2DComponent>(collider3D->collidedEntity);
			if (hit2D && hit2D->collisionLayer == CommentBout::CollisionLayer::CBPlayerAttack) {
				isHitByPlayerAttack = true;
				auto* damage = registry.GetComponent<AttackDamageComponent>(collider3D->collidedEntity);
				if (damage) {
					attackDamage = std::max(1, damage->damage);
				}
			}
		}

		if (isHitByPlayerAttack && !enemy->wasCollidingWithAttack) {
			enemy->hp -= attackDamage;
			enemy->lastDamageTaken = attackDamage;
			enemy->damageFlashTimer = enemy->damageFlashDuration;
			if (enemy->hp < 0) {
				enemy->hp = 0;
			}
		}
		enemy->wasCollidingWithAttack = isHitByPlayerAttack;

		bool isTouchingPlayerHitbox = false;
		auto hitboxView = registry.View<CBPlayerHitboxTag, PlayerHitboxComponent, CommentBoutCollision::Collider3DComponent>();
		for (auto hitboxEntity : hitboxView) {
			auto* hitbox = registry.GetComponent<PlayerHitboxComponent>(hitboxEntity);
			auto* playerCollider = registry.GetComponent<CommentBoutCollision::Collider3DComponent>(hitboxEntity);
			if (!hitbox || !playerCollider) {
				continue;
			}
			if (CheckEnemyVsPlayerHitbox(*collider3D, *playerCollider, *hitbox)) {
				isTouchingPlayerHitbox = true;
				auto* health = registry.GetComponent<PlayerHealthComponent>(hitbox->playerEntity);
				if (health && !health->isDead && health->invincibleTime <= 0.0f && !enemy->wasCollidingWithPlayer) {
					health->hp -= 1;
					if (health->hp < 0) {
						health->hp = 0;
					}
					health->lastDamageTaken = 1;
					health->invincibleTime = health->invincibleDuration;
					if (health->hp <= 0) {
						health->isDead = true;
					}
				}
			}
		}
		enemy->wasCollidingWithPlayer = isTouchingPlayerHitbox;

		if (enemy->damageFlashTimer > 0.0f) {
			enemy->damageFlashTimer -= deltaTime;
			material->color = { 1.0f, 0.2f, 0.2f, 1.0f };
		} else {
			enemy->damageFlashTimer = 0.0f;
			material->color = { 1.0f, 1.0f, 1.0f, 1.0f };
		}

#ifdef USE_IMGUI
		ImGui::Text("Enemy %llu hp=%d/%d atkHit=%s flash=%.2f playerHit=%s", static_cast<unsigned long long>(entity), enemy->hp, enemy->maxHp, isHitByPlayerAttack ? "true" : "false", enemy->damageFlashTimer, isTouchingPlayerHitbox ? "true" : "false");
		ImGui::Text("Collider pos(%.2f, %.2f, %.2f)", collider3D->worldPosition.x, collider3D->worldPosition.y, collider3D->worldPosition.z);
		if (collider3D->shapeType == CommentBoutCollision::ShapeType3D::Box) {
			ImGui::Text("Collider box(%.2f, %.2f, %.2f)", collider3D->worldBoxSize.x, collider3D->worldBoxSize.y, collider3D->worldBoxSize.z);
		} else {
			ImGui::Text("Collider radius=%.2f", collider3D->worldRadius);
		}
		ImGui::Separator();
#endif

		if (drawColliderDebug) {
			const NoEngine::Math::Color color = collider3D->isColliding ? NoEngine::Math::Color(1.0f, 0.0f, 0.0f, 1.0f) : NoEngine::Math::Color(1.0f, 1.0f, 1.0f, 1.0f);
			DrawColliderDebug(*collider3D, color);
		}

		if (enemy->hp <= 0) {
			registry.DestroyEntity(entity);
		}
	}

#ifdef USE_IMGUI
	ImGui::End();
#endif
}



