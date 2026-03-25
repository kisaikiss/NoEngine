#include "stdafx.h"
#include "DamageApplySystem.h"
#include "application/CommentBout/Component/DamageRequestComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Component/InvincibleComponent.h"
#include "application/CommentBout/Component/PlayerHealthComponent.h"
#include "application/CommentBout/Component/EnemyComponent.h"
#include "application/CommentBout/GameTag.h"
#include <algorithm>

namespace {
void ApplyToHealth(HealthComponent* health, int amount) {
	if (!health) {
		return;
	}
	if (health->isDead || amount <= 0) {
		return;
	}

	health->hp -= amount;
	if (health->hp < 0) {
		health->hp = 0;
	}
	health->lastDamageTaken = amount;
	health->isDead = (health->hp <= 0);
}

void SyncToLegacyPlayerHealth(PlayerHealthComponent* legacy, const HealthComponent* health) {
	if (!legacy || !health) {
		return;
	}
	legacy->hp = health->hp;
	legacy->maxHp = health->maxHp;
	legacy->isDead = health->isDead;
	legacy->deathHandled = health->deathHandled;
	legacy->lastDamageTaken = health->lastDamageTaken;
}

void SyncToLegacyEnemy(EnemyComponent* legacy, const HealthComponent* health) {
	if (!legacy || !health) {
		return;
	}
	legacy->hp = health->hp;
	legacy->maxHp = health->maxHp;
	legacy->lastDamageTaken = health->lastDamageTaken;
}
}

void DamageApplySystem::Update(No::Registry& registry, float deltaTime)
{
	// 無敵時間を持つ対象はここで減算。
	// 将来的に PlayerControl から段階移行するため、まずは共通System側でも更新する。
	{
		auto invView = registry.View<InvincibleComponent>();
		for (auto entity : invView) {
			auto* inv = registry.GetComponent<InvincibleComponent>(entity);
			if (!inv) {
				continue;
			}
			if (inv->time > 0.0f) {
				inv->time -= deltaTime;
				if (inv->time < 0.0f) {
					inv->time = 0.0f;
				}
			}
		}
	}

	// リクエストは即時Destroyせず、最後にまとめて破棄する。
	std::vector<No::Entity> consumedRequests;

	auto requestView = registry.View<DamageRequestComponent>();
	for (auto requestEntity : requestView) {
		auto* request = registry.GetComponent<DamageRequestComponent>(requestEntity);
		if (!request) {
			continue;
		}

		if (request->target == No::nullEntity || request->amount <= 0) {
			consumedRequests.push_back(requestEntity);
			continue;
		}

		auto* health = registry.GetComponent<HealthComponent>(request->target);
		InvincibleComponent* inv = nullptr;
		if (registry.Has<InvincibleComponent>(request->target)) {
			inv = registry.GetComponent<InvincibleComponent>(request->target);
		}

		const bool isInvincible = (inv && inv->time > 0.0f);
		if (isInvincible && !request->ignoreInvincible) {
			consumedRequests.push_back(requestEntity);
			continue;
		}

		ApplyToHealth(health, request->amount);

		if (health && health->isDead && registry.Has<CBRailEnemyTag>(request->target)) {
			registry.DestroyEntity(request->target);
		}

		// 互換維持: 既存の個別コンポーネントにも反映。
		auto* legacyPlayerHealth = registry.GetComponent<PlayerHealthComponent>(request->target);
		if (legacyPlayerHealth) {
			if (inv && request->amount > 0) {
				inv->duration = legacyPlayerHealth->invincibleDuration;
				inv->time = std::max(inv->time, inv->duration);
				legacyPlayerHealth->invincibleTime = std::max(legacyPlayerHealth->invincibleTime, legacyPlayerHealth->invincibleDuration);
			}
			SyncToLegacyPlayerHealth(legacyPlayerHealth, health);
		}

		auto* legacyEnemy = registry.GetComponent<EnemyComponent>(request->target);
		if (legacyEnemy) {
			SyncToLegacyEnemy(legacyEnemy, health);
		}

		consumedRequests.push_back(requestEntity);
	}

	for (auto requestEntity : consumedRequests) {
		registry.DestroyEntity(requestEntity);
	}
}
