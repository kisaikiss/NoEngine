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

namespace {
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
}

void EnemyContactDamageSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);

	No::TransformComponent* activeCameraTransform = nullptr;
	auto activeCameraView = registry.View<No::CameraComponent, No::TransformComponent, No::ActiveCameraTag>();
	auto activeCameraIt = activeCameraView.begin();
	if (activeCameraIt != activeCameraView.end()) {
		activeCameraTransform = registry.GetComponent<No::TransformComponent>(*activeCameraIt);
	}

	auto enemyView = registry.View<CBRailEnemyTag, EnemyComponent, HealthComponent, CommentBoutCollision::Collider3DComponent>();
	for (auto entity : enemyView) {
		auto* enemy = registry.GetComponent<EnemyComponent>(entity);
		auto* health = registry.GetComponent<HealthComponent>(entity);
		auto* collider3D = registry.GetComponent<CommentBoutCollision::Collider3DComponent>(entity);
		auto* enemyProjected = registry.GetComponent<CommentBoutCollision::ProjectedColliderComponent>(entity);
		if (!enemy || !health || !collider3D || !enemyProjected) {
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
			if (!hitbox || hitbox->playerEntity == No::nullEntity) {
				continue;
			}

			auto* playerTransform2D = registry.GetComponent<No::Transform2DComponent>(hitbox->playerEntity);
			if (!playerTransform2D || !activeCameraTransform) {
				continue;
			}

			CommentBoutCollision::CameraGateParams gate{};
			gate.useCameraGate = hitbox->useCameraGateForPlayerHit;
			gate.nearZ = hitbox->cameraGateNear;
			gate.depth = hitbox->cameraGateDepth;
			gate.halfWidth = hitbox->cameraGateHalfWidth;
			gate.halfHeight = hitbox->cameraGateHalfHeight;

			const bool hit = CommentBoutCollision::CollisionAlgorithms::CheckProjectedVsSpriteAndCameraGate(
				*enemyProjected,
				collider3D->worldPosition,
				*playerTransform2D,
				*activeCameraTransform,
				gate
			);
			if (!hit) {
				continue;
			}

			isTouchingPlayerHitbox = true;
			auto* playerHealth = registry.GetComponent<HealthComponent>(hitbox->playerEntity);
			auto* playerInvincible = registry.GetComponent<InvincibleComponent>(hitbox->playerEntity);
			const bool canTakeDamage =
				playerHealth &&
				!playerHealth->isDead &&
				(!playerInvincible || playerInvincible->time <= 0.0f);

			if (canTakeDamage && !enemy->wasCollidingWithPlayer) {
				EmitDamageRequest(registry, hitbox->playerEntity, entity, 1, false);
			}
		}

		enemy->wasCollidingWithPlayer = isTouchingPlayerHitbox;
	}

	// Phase14コメント:
	// 敵接触被弾も敵弾被弾と同じ共通判定関数を使い、条件不一致をなくしている。
}
