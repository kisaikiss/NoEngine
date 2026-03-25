#include "stdafx.h"
#include "EnemyRewardToBossSystem.h"
#include "application/CommentBout/Component/EnemyRewardOrbComponent.h"
#include "application/CommentBout/Component/BossHpBarComponent.h"
#include "application/CommentBout/Component/DamageRequestComponent.h"
#include "application/CommentBout/GameTag.h"
#include <algorithm>

namespace {
No::Vector2 Bezier2(const No::Vector2& p0, const No::Vector2& p1, const No::Vector2& p2, float t) {
	const float u = 1.0f - t;
	return p0 * (u * u) + p1 * (2.0f * u * t) + p2 * (t * t);
}
}

void EnemyRewardToBossSystem::Update(No::Registry& registry, float deltaTime)
{
	No::Entity bossEntity = No::nullEntity;
	auto barView = registry.View<CBBossHpBarTag, BossHpBarComponent>();
	for (auto e : barView) {
		auto* bar = registry.GetComponent<BossHpBarComponent>(e);
		if (bar && bar->bossEntity != No::nullEntity) {
			bossEntity = bar->bossEntity;
			break;
		}
	}

	auto rewardView = registry.View<CBEnemyRewardOrbTag, EnemyRewardOrbComponent, No::Transform2DComponent>();
	for (auto entity : rewardView) {
		auto* reward = registry.GetComponent<EnemyRewardOrbComponent>(entity);
		auto* t = registry.GetComponent<No::Transform2DComponent>(entity);
		if (!reward || !t) {
			continue;
		}

		reward->elapsed += deltaTime;
		const float duration = std::max(0.05f, reward->duration);
		const float rate = std::min(1.0f, reward->elapsed / duration);
		t->translate = Bezier2(reward->start, reward->control, reward->end, rate);

		if (rate < 1.0f) {
			continue;
		}

		if (bossEntity != No::nullEntity) {
			auto req = registry.GenerateEntity();
			auto* damage = registry.AddComponent<DamageRequestComponent>(req);
			damage->target = bossEntity;
			damage->amount = std::max(1, reward->attackPower);
			damage->ignoreInvincible = true;
		}

		registry.DestroyEntity(entity);
	}
}
