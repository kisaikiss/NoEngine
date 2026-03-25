#include "stdafx.h"
#include "EnemyContactDamageSystem.h"
#include "application/CommentBout/Component/EnemyComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Component/InvincibleComponent.h"
#include "application/CommentBout/Component/DamageRequestComponent.h"
#include "application/CommentBout/Component/PlayerHitboxComponent.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Collision/Component/Collider3DComponent.h"
#include "application/CommentBout/Collision/Component/ProjectedColliderComponent.h"
#include "application/CommentBout/Collision/Utility/CollisionAlgorithms.h"
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

bool CheckEnemyInCameraGate(
	const No::Vector3& enemyWorldPos,
	No::TransformComponent& cameraTransform,
	const PlayerHitboxComponent& hitbox
) {
	No::Matrix4x4 cameraWorld = cameraTransform.MakeAffineMatrix4x4();
	const No::Vector3 cameraPos = cameraTransform.GetWorldPosition();
	No::Vector3 right = NormalizeOrDefault(cameraWorld.TransformNormal(No::Vector3::RIGHT), No::Vector3::RIGHT);
	No::Vector3 up = NormalizeOrDefault(cameraWorld.TransformNormal(No::Vector3::UP), No::Vector3::UP);
	No::Vector3 forward = NormalizeOrDefault(cameraWorld.TransformNormal(No::Vector3::FORWARD), No::Vector3::FORWARD);

	const No::Vector3 toEnemy = enemyWorldPos - cameraPos;
	const float camX = toEnemy.Dot(right);
	const float camY = toEnemy.Dot(up);
	const float camZ = toEnemy.Dot(forward);

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
}

void EnemyContactDamageSystem::Update(No::Registry& registry, float deltaTime)
{
	static float playerFlashTimer = 0.0f;
	static_cast<void>(deltaTime);

	{
		auto playerView = registry.View<CBPlayerTag, No::SpriteComponent>();
		for (auto e : playerView) {
			auto* sprite = registry.GetComponent<No::SpriteComponent>(e);
			if (!sprite) {
				continue;
			}
			if (playerFlashTimer > 0.0f) {
				playerFlashTimer -= deltaTime;
				sprite->color = { 1.0f, 0.35f, 0.35f, 0.75f };
			} else {
				sprite->color = { 1.0f, 1.0f, 1.0f, 0.5f };
			}
		}
	}

	No::CameraComponent* activeCamera = nullptr;
	No::TransformComponent* activeCameraTransform = nullptr;
	auto activeCameraView = registry.View<No::CameraComponent, No::TransformComponent, No::ActiveCameraTag>();
	auto activeCameraIt = activeCameraView.begin();
	if (activeCameraIt != activeCameraView.end()) {
		activeCamera = registry.GetComponent<No::CameraComponent>(*activeCameraIt);
		activeCameraTransform = registry.GetComponent<No::TransformComponent>(*activeCameraIt);
	}

	auto enemyView = registry.View<CBRailEnemyTag, EnemyComponent, HealthComponent, CommentBoutCollision::Collider3DComponent>();
	for (auto entity : enemyView) {
		auto* enemy = registry.GetComponent<EnemyComponent>(entity);
		auto* health = registry.GetComponent<HealthComponent>(entity);
		auto* collider3D = registry.GetComponent<CommentBoutCollision::Collider3DComponent>(entity);
		auto* enemyProjected = registry.GetComponent<CommentBoutCollision::ProjectedColliderComponent>(entity);
		if (!enemy || !health || !collider3D) {
			continue;
		}
		if (health->isDead || health->hp <= 0) {
			enemy->wasCollidingWithPlayer = false;
			continue;
		}

		bool isTouchingPlayerHitbox = false;
		auto hitboxView = registry.View<CBPlayerHitboxTag, PlayerHitboxComponent>();
		for (auto hitboxEntity : hitboxView) {
			auto* hitbox = registry.GetComponent<PlayerHitboxComponent>(hitboxEntity);
			if (!hitbox) {
				continue;
			}

			bool hit = false;
			if (hitbox->useCameraGateForPlayerHit && enemyProjected && activeCamera && activeCameraTransform) {
				auto* playerTransform2D = registry.GetComponent<No::Transform2DComponent>(hitbox->playerEntity);
				if (playerTransform2D) {
					const bool spriteOverlap = CheckProjectedVsPlayerSprite(*enemyProjected, *playerTransform2D);
					const bool spriteGatePass = CheckEnemyInCameraGate(collider3D->worldPosition, *activeCameraTransform, *hitbox);
					hit = spriteOverlap && spriteGatePass;
				}
			}

			if (hit) {
				isTouchingPlayerHitbox = true;
				auto* playerHealth = registry.GetComponent<HealthComponent>(hitbox->playerEntity);
				auto* playerInvincible = registry.GetComponent<InvincibleComponent>(hitbox->playerEntity);
				const bool canTakeDamage =
					playerHealth &&
					!playerHealth->isDead &&
					(!playerInvincible || playerInvincible->time <= 0.0f);

				if (canTakeDamage && !enemy->wasCollidingWithPlayer) {
					EmitDamageRequest(registry, hitbox->playerEntity, entity, 1, false);
					playerFlashTimer = 0.16f;
				}
			}
		}

		enemy->wasCollidingWithPlayer = isTouchingPlayerHitbox;
	}
}
