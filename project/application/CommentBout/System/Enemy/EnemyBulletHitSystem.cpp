#include "stdafx.h"
#include "EnemyBulletHitSystem.h"
#include "application/CommentBout/Component/Enemy/EnemyBulletComponent.h"
#include "application/CommentBout/Component/DamageRequestComponent.h"
#include "application/CommentBout/Component/Player/PlayerHitboxComponent.h"
#include "application/CommentBout/Collision/Component/Collider3DComponent.h"
#include "application/CommentBout/Collision/Component/ProjectedColliderComponent.h"
#include "application/CommentBout/Collision/Utility/CollisionAlgorithms.h"
#include "application/CommentBout/Utility/EnemyVisibilityUtility.h"
#include "application/CommentBout/GameTag.h"
#include "engine/Functions/ECS/Component/CameraComponent.h"
#include "engine/Functions/ECS/Component/Transform2DComponent.h"
#include <algorithm>

bool EnemyBulletHitSystem::IsPlayerHitByUnifiedRule(
	No::Registry& registry,
	No::Entity bulletEntity,
	const CommentBoutCollision::Collider3DComponent& bulletCollider,
	No::Entity& outPlayerEntity
) const {
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
	if (!activeCameraTransform) {
		return false;
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

		CommentBoutCollision::CameraGateParams gate{};
		gate.useCameraGate = hitbox->useCameraGateForPlayerHit;
		gate.nearZ = hitbox->cameraGateNear;
		gate.depth = hitbox->cameraGateDepth;
		gate.halfWidth = hitbox->cameraGateHalfWidth;
		gate.halfHeight = hitbox->cameraGateHalfHeight;

		const bool hit = CommentBoutCollision::CollisionAlgorithms::CheckProjectedVsSpriteAndCameraGate(
			*projected,
			bulletCollider.worldPosition,
			*playerTransform2D,
			*activeCameraTransform,
			gate
		);
		if (hit) {
			outPlayerEntity = hitbox->playerEntity;
			return true;
		}
	}

	return false;
}

bool EnemyBulletHitSystem::IsFieldHitByBullet(
	No::Registry& registry,
	No::Entity bulletEntity,
	const CommentBoutCollision::Collider3DComponent& bulletCollider
) const {
	for (auto fieldEntity : registry.View<CBFieldObjectTag, CommentBoutCollision::Collider3DComponent>()) {
		if (fieldEntity == bulletEntity) {
			continue;
		}

		auto* fieldCollider = registry.GetComponent<CommentBoutCollision::Collider3DComponent>(fieldEntity);
		if (!fieldCollider) {
			continue;
		}

		bool hit = false;
		if (bulletCollider.shapeType == CommentBoutCollision::ShapeType3D::Sphere &&
			fieldCollider->shapeType == CommentBoutCollision::ShapeType3D::Box) {
			hit = CommentBoutCollision::CollisionAlgorithms::CheckSphereAABB3D(
				bulletCollider.worldPosition,
				bulletCollider.worldRadius,
				fieldCollider->worldPosition,
				fieldCollider->worldBoxSize
			);
		} else if (bulletCollider.shapeType == CommentBoutCollision::ShapeType3D::Sphere &&
			fieldCollider->shapeType == CommentBoutCollision::ShapeType3D::Sphere) {
			hit = CommentBoutCollision::CollisionAlgorithms::CheckSphereSphere(
				bulletCollider.worldPosition,
				bulletCollider.worldRadius,
				fieldCollider->worldPosition,
				fieldCollider->worldRadius
			);
		} else {
			hit = CommentBoutCollision::CollisionAlgorithms::CheckAABB3DAABB3D(
				bulletCollider.worldPosition,
				bulletCollider.worldBoxSize,
				fieldCollider->worldPosition,
				fieldCollider->worldBoxSize
			);
		}

		if (hit) {
			return true;
		}
	}

	return false;
}

void EnemyBulletHitSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);

	No::TransformComponent* activeCameraTransform = nullptr;
	{
		auto cameraView = registry.View<No::CameraComponent, No::TransformComponent, No::ActiveCameraTag>();
		auto cameraIt = cameraView.begin();
		if (cameraIt != cameraView.end()) {
			activeCameraTransform = registry.GetComponent<No::TransformComponent>(*cameraIt);
		}
	}

	auto view = registry.View<CBEnemyBulletTag, EnemyBulletComponent, CommentBoutCollision::Collider3DComponent, No::TransformComponent>();
	for (auto bulletEntity : view) {
		auto* bullet = registry.GetComponent<EnemyBulletComponent>(bulletEntity);
		auto* collider = registry.GetComponent<CommentBoutCollision::Collider3DComponent>(bulletEntity);
		auto* bulletTransform = registry.GetComponent<No::TransformComponent>(bulletEntity);
		if (!bullet || !collider || !bulletTransform) {
			continue;
		}

		if (activeCameraTransform) {
			const float forwardDistance = CommentBoutVisibility::ComputeForwardDistanceFromCamera(*bulletTransform, *activeCameraTransform);
			if (forwardDistance < -10.0f) {
				registry.DestroyEntity(bulletEntity);
				continue;
			}
		}

		No::Entity playerEntity = No::nullEntity;
		const bool hitPlayer = IsPlayerHitByUnifiedRule(registry, bulletEntity, *collider, playerEntity);
		const bool hitField = IsFieldHitByBullet(registry, bulletEntity, *collider);

		if (hitPlayer && playerEntity != No::nullEntity) {
			auto req = registry.GenerateEntity();
			auto* damage = registry.AddComponent<DamageRequestComponent>(req);
			damage->target = playerEntity;
			damage->amount = std::max(1, bullet->damage);
			damage->ignoreInvincible = false;
		}

		if (hitPlayer || hitField) {
			// 自機被弾とフィールド衝突を明示的に分離し、広域isColliding依存での誤消滅を防ぐ。
			registry.DestroyEntity(bulletEntity);
		}
	}

	// Phase14コメント:
	// 自機被弾は「投影重なり + カメラゲート」に一本化し、判定ぶれをなくしている。
}
