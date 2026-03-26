#include "stdafx.h"
#include "EnemyBulletHitSystem.h"
#include "application/CommentBout/Component/EnemyBulletComponent.h"
#include "application/CommentBout/Component/DamageRequestComponent.h"
#include "application/CommentBout/Component/PlayerHitboxComponent.h"
#include "application/CommentBout/Collision/Component/Collider3DComponent.h"
#include "application/CommentBout/Collision/Component/ProjectedColliderComponent.h"
#include "application/CommentBout/Collision/Utility/CollisionAlgorithms.h"
#include "application/CommentBout/GameTag.h"
#include "engine/Functions/ECS/Component/CameraComponent.h"
#include "engine/Functions/ECS/Component/Transform2DComponent.h"
#include <algorithm>
#include <cmath>

namespace {
No::Vector3 NormalizeOrDefault(const No::Vector3& v, const No::Vector3& fallback) {
	if (v.LengthSquared() <= 0.000001f) {
		return fallback;
	}
	return v.Normalize();
}

bool CheckProjectedVsPlayerSprite(
	const CommentBoutCollision::ProjectedColliderComponent& projected,
	const No::Transform2DComponent& playerTransform2D
) {
	if (!projected.isVisible) {
		return false;
	}

	if (projected.isBox) {
		return CommentBoutCollision::CollisionAlgorithms::CheckConvexHullAABB(
			projected.convexHull,
			playerTransform2D.translate,
			playerTransform2D.scale
		);
	}

	return CommentBoutCollision::CollisionAlgorithms::CheckCircleAABB(
		projected.screenPosition,
		projected.screenRadius,
		playerTransform2D.translate,
		playerTransform2D.scale
	);
}

bool CheckInCameraGate(
	const No::Vector3& worldPos,
	No::TransformComponent& cameraTransform,
	const PlayerHitboxComponent& hitbox
) {
	No::Matrix4x4 cameraWorld = cameraTransform.MakeAffineMatrix4x4();
	const No::Vector3 cameraPos = cameraTransform.GetWorldPosition();
	No::Vector3 right = NormalizeOrDefault(cameraWorld.TransformNormal(No::Vector3::RIGHT), No::Vector3::RIGHT);
	No::Vector3 up = NormalizeOrDefault(cameraWorld.TransformNormal(No::Vector3::UP), No::Vector3::UP);
	No::Vector3 forward = NormalizeOrDefault(cameraWorld.TransformNormal(No::Vector3::FORWARD), No::Vector3::FORWARD);

	const No::Vector3 toTarget = worldPos - cameraPos;
	const float camX = toTarget.Dot(right);
	const float camY = toTarget.Dot(up);
	const float camZ = toTarget.Dot(forward);

	const float nearZ = std::max(0.0f, hitbox.cameraGateNear);
	const float depth = std::max(0.001f, hitbox.cameraGateDepth);
	const float halfW = std::max(0.001f, hitbox.cameraGateHalfWidth);
	const float halfH = std::max(0.001f, hitbox.cameraGateHalfHeight);

	if (camZ < nearZ || camZ > nearZ + depth) {
		return false;
	}
	if (std::abs(camX) > halfW) {
		return false;
	}
	if (std::abs(camY) > halfH) {
		return false;
	}
	return true;
}

bool IsBulletHitPlayerByProjected(
	No::Registry& registry,
	No::Entity bulletEntity,
	const CommentBoutCollision::Collider3DComponent& bulletCollider,
	No::Entity& outPlayerEntity
) {
	outPlayerEntity = No::nullEntity;
	auto* projected = registry.GetComponent<CommentBoutCollision::ProjectedColliderComponent>(bulletEntity);
	if (!projected) {
		return false;
	}

	No::TransformComponent* activeCameraTransform = nullptr;
	auto cameraView = registry.View<No::CameraComponent, No::TransformComponent, No::ActiveCameraTag>();
	auto cameraIt = cameraView.begin();
	if (cameraIt != cameraView.end()) {
		activeCameraTransform = registry.GetComponent<No::TransformComponent>(*cameraIt);
	}

	auto hitboxView = registry.View<CBPlayerHitboxTag, PlayerHitboxComponent>();
	for (auto hitboxEntity : hitboxView) {
		auto* hitbox = registry.GetComponent<PlayerHitboxComponent>(hitboxEntity);
		if (!hitbox || hitbox->playerEntity == No::nullEntity) {
			continue;
		}

		auto* playerTransform2D = registry.GetComponent<No::Transform2DComponent>(hitbox->playerEntity);
		if (!playerTransform2D) {
			continue;
		}

		const bool spriteOverlap = CheckProjectedVsPlayerSprite(*projected, *playerTransform2D);
		if (!spriteOverlap) {
			continue;
		}

		bool gatePass = true;
		if (hitbox->useCameraGateForPlayerHit && activeCameraTransform) {
			gatePass = CheckInCameraGate(bulletCollider.worldPosition, *activeCameraTransform, *hitbox);
		}
		if (!gatePass) {
			continue;
		}

		outPlayerEntity = hitbox->playerEntity;
		return true;
	}

	return false;
}
}

void EnemyBulletHitSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);

	auto view = registry.View<CBEnemyBulletTag, EnemyBulletComponent, CommentBoutCollision::Collider3DComponent>();
	for (auto bulletEntity : view) {
		auto* bullet = registry.GetComponent<EnemyBulletComponent>(bulletEntity);
		auto* collider = registry.GetComponent<CommentBoutCollision::Collider3DComponent>(bulletEntity);
		if (!bullet || !collider) {
			continue;
		}

		No::Entity playerEntity = No::nullEntity;
		// 被弾判定は照準点（targetDepthFromCamera）を使わず、
		// 接触判定と同じ投影+ゲート方式で判定する。
		const bool hitPlayer = IsBulletHitPlayerByProjected(registry, bulletEntity, *collider, playerEntity);

		if (hitPlayer && playerEntity != No::nullEntity) {
			auto req = registry.GenerateEntity();
			auto* damage = registry.AddComponent<DamageRequestComponent>(req);
			damage->target = playerEntity;
			damage->amount = std::max(1, bullet->damage);
			damage->ignoreInvincible = false;
		}

		if (hitPlayer || collider->isColliding) {
			// 敵弾は「時間経過(LifetimeSystem)」または「衝突時」に消える。
			registry.DestroyEntity(bulletEntity);
		}
	}
}
