#include "stdafx.h"
#include "SpeechBubbleToBossSystem.h"
#include "application/CommentBout/Component/Enemy/SpeechBubbleComponent.h"
#include "application/CommentBout/Component/HpBarComponent.h"
#include "application/CommentBout/Component/DamageRequestComponent.h"
#include "application/CommentBout/GameTag.h"
#include <algorithm>

namespace {
No::Vector2 Bezier2(const No::Vector2& p0, const No::Vector2& p1, const No::Vector2& p2, float t) {
	const float u = 1.0f - t;
	return p0 * (u * u) + p1 * (2.0f * u * t) + p2 * (t * t);
}
}

void SpeechBubbleToBossSystem::Update(No::Registry& registry, float deltaTime)
{
	No::Entity bossEntity = No::nullEntity;
	auto barView = registry.View<CBBossHpBarTag, HpBarComponent>();
	for (auto e : barView) {
		auto* bar = registry.GetComponent<HpBarComponent>(e);
		if (bar && bar->targetEntity != No::nullEntity && registry.Has<CBBossTag>(bar->targetEntity)) {
			bossEntity = bar->targetEntity;
			break;
		}
	}

	auto rewardView = registry.View<CBSpeechBubbleTag, SpeechBubbleComponent, No::Transform2DComponent>();
	for (auto entity : rewardView) {
		auto* bubble = registry.GetComponent<SpeechBubbleComponent>(entity);
		auto* t      = registry.GetComponent<No::Transform2DComponent>(entity);
		if (!bubble || !t) {
			continue;
		}

		bubble->elapsed += deltaTime;
		const float duration = std::max(0.05f, bubble->duration);
		const float rate     = std::min(1.0f, bubble->elapsed / duration);
		t->translate = Bezier2(bubble->start, bubble->control, bubble->end, rate);

		if (rate < 1.0f) {
			continue;
		}

		if (bossEntity != No::nullEntity) {
			auto req    = registry.GenerateEntity();
			auto* damage = registry.AddComponent<DamageRequestComponent>(req);
			damage->target          = bossEntity;
			damage->amount          = std::max(1, bubble->attackPower);
			damage->ignoreInvincible = true;
		}

		registry.DestroyEntity(entity);
	}
}
