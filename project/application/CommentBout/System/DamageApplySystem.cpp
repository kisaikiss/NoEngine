#include "stdafx.h"
#include "DamageApplySystem.h"
#include "application/CommentBout/Component/DamageRequestComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Component/InvincibleComponent.h"
#include "application/CommentBout/Component/EnemyComponent.h"
#include "application/CommentBout/Component/DamageFlashComponent.h"
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

void SyncToLegacyEnemy(EnemyComponent* legacy, const HealthComponent* health) {
	if (!legacy || !health) {
		return;
	}
	legacy->hp = health->hp;
	legacy->maxHp = health->maxHp;
	legacy->lastDamageTaken = health->lastDamageTaken;
}

void ApplyDamageFlash(No::Registry& registry, No::Entity target) {
	if (target == No::nullEntity) {
		return;
	}

	if (!registry.Has<DamageFlashComponent>(target)) {
		registry.AddComponent<DamageFlashComponent>(target);
	}
	auto* flash = registry.GetComponent<DamageFlashComponent>(target);
	if (!flash) {
		return;
	}

	if (registry.Has<CBPlayerTag>(target)) {
		flash->duration = 0.16f;
		flash->flashColor = No::Color(1.0f, 0.35f, 0.35f, 0.75f);
		flash->baseColor = No::Color(1.0f, 1.0f, 1.0f, 0.5f);
		flash->affectSprite = true;
		flash->affectMaterial = false;
	} else {
		flash->duration = 0.12f;
		flash->flashColor = No::Color(1.0f, 0.2f, 0.2f, 1.0f);
		flash->baseColor = No::Color(1.0f, 1.0f, 1.0f, 1.0f);
		flash->affectSprite = false;
		flash->affectMaterial = true;
	}
	flash->timer = std::max(flash->timer, flash->duration);
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

		const bool useInvincible = registry.Has<CBPlayerTag>(request->target);
		const bool isInvincible = useInvincible && (inv && inv->time > 0.0f);
		if (isInvincible && !request->ignoreInvincible) {
			consumedRequests.push_back(requestEntity);
			continue;
		}

		ApplyToHealth(health, request->amount);
		if (health && request->amount > 0 && !health->isDead) {
			ApplyDamageFlash(registry, request->target);
		}

		auto* legacyEnemy = registry.GetComponent<EnemyComponent>(request->target);
		if (health && health->isDead && legacyEnemy) {
			legacyEnemy->removeReason = EnemyRemoveReason::Defeated;
		}
		if (health && health->isDead && registry.Has<CBRailEnemyTag>(request->target)) {
			registry.DestroyEntity(request->target);
		}

		if (registry.Has<CBPlayerTag>(request->target) && inv && request->amount > 0) {
			inv->time = std::max(inv->time, inv->duration);
		}

		if (legacyEnemy) {
			SyncToLegacyEnemy(legacyEnemy, health);
		}

		consumedRequests.push_back(requestEntity);
	}

	for (auto requestEntity : consumedRequests) {
		registry.DestroyEntity(requestEntity);
	}
}
