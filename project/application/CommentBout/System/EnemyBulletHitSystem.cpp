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
	static_cast<void>(deltaTime);

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
		}

		registry.DestroyEntity(bulletEntity);
	}
}
