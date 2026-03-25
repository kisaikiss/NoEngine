#include "stdafx.h"
#include "EnemyBulletHitSystem.h"
#include "application/CommentBout/Component/EnemyBulletComponent.h"
#include "application/CommentBout/Component/DamageRequestComponent.h"
#include "application/CommentBout/Component/PlayerHitboxComponent.h"
#include "application/CommentBout/Collision/Component/Collider3DComponent.h"
#include "application/CommentBout/GameTag.h"
#include <algorithm>

void EnemyBulletHitSystem::Update(No::Registry& registry, float deltaTime)
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
			}
		}
	}

	auto view = registry.View<CBEnemyBulletTag, EnemyBulletComponent, CommentBoutCollision::Collider3DComponent>();
	for (auto bulletEntity : view) {
		auto* bullet = registry.GetComponent<EnemyBulletComponent>(bulletEntity);
		auto* collider = registry.GetComponent<CommentBoutCollision::Collider3DComponent>(bulletEntity);
		if (!bullet || !collider) {
			continue;
		}
		if (!collider->isColliding) {
			continue;
		}

		No::Entity target = No::nullEntity;
		if (registry.Has<CBPlayerHitboxTag>(collider->collidedEntity)) {
			auto* hitbox = registry.GetComponent<PlayerHitboxComponent>(collider->collidedEntity);
			if (hitbox) {
				target = hitbox->playerEntity;
			}
		}

		if (target != No::nullEntity) {
			auto req = registry.GenerateEntity();
			auto* damage = registry.AddComponent<DamageRequestComponent>(req);
			damage->target = target;
			damage->amount = std::max(1, bullet->damage);
			damage->ignoreInvincible = false;
			playerFlashTimer = 0.16f;
		}

		registry.DestroyEntity(bulletEntity);
	}
}
