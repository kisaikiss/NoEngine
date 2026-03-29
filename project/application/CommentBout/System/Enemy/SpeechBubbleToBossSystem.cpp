#include "stdafx.h"
#include "SpeechBubbleToBossSystem.h"
#include "application/CommentBout/Component/Enemy/SpeechBubbleComponent.h"
#include "application/CommentBout/Component/HpBarComponent.h"
#include "application/CommentBout/Component/DamageRequestComponent.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/Data/SpeechBubbleConfig.h"
#include "application/CommentBout/GameTag.h"
#include <algorithm>

namespace {

No::Vector2 Bezier2(const No::Vector2& p0, const No::Vector2& p1, const No::Vector2& p2, float t) {
	const float u = 1.0f - t;
	return p0 * (u * u) + p1 * (2.0f * u * t) + p2 * (t * t);
}

// バウンスイージング（0→1 の出力範囲で 0→1 に収束する）
float EaseBounceOut(float t) {
	constexpr float n1 = 7.5625f, d1 = 2.75f;
	if (t < 1.0f / d1) {
		return n1 * t * t;
	} else if (t < 2.0f / d1) {
		t -= 1.500f / d1;
		return n1 * t * t + 0.7500f;
	} else if (t < 2.5f / d1) {
		t -= 2.250f / d1;
		return n1 * t * t + 0.9375f;
	} else {
		t -= 2.625f / d1;
		return n1 * t * t + 0.984375f;
	}
}

No::Vector2 LerpV2(const No::Vector2& a, const No::Vector2& b, float t) {
	return a + (b - a) * t;
}

} // namespace

void SpeechBubbleToBossSystem::Update(No::Registry& registry, float deltaTime)
{
	// ボスエンティティを取得
	No::Entity bossEntity = No::nullEntity;
	auto barView = registry.View<CBBossHpBarTag, HpBarComponent>();
	for (auto e : barView) {
		auto* bar = registry.GetComponent<HpBarComponent>(e);
		if (bar && bar->targetEntity != No::nullEntity && registry.Has<CBBossTag>(bar->targetEntity)) {
			bossEntity = bar->targetEntity;
			break;
		}
	}

	// SpeechBubbleConfig を取得
	const SpeechBubbleConfig* sbConfig = nullptr;
	static SpeechBubbleConfig sDefault;
	for (auto e : registry.View<CBSpeechBubbleConfigTag, SpeechBubbleConfig>()) {
		sbConfig = registry.GetComponent<SpeechBubbleConfig>(e);
		if (sbConfig) break;
	}
	if (!sbConfig) { sbConfig = &sDefault; }

	// 削除待ちエンティティを一時収集（イテレーション中に破棄しない）
	std::vector<No::Entity> toDestroy;

	auto rewardView = registry.View<CBSpeechBubbleTag, SpeechBubbleComponent, No::Transform2DComponent>();
	for (auto entity : rewardView) {
		auto* bubble = registry.GetComponent<SpeechBubbleComponent>(entity);
		auto* t      = registry.GetComponent<No::Transform2DComponent>(entity);
		if (!bubble || !t) {
			continue;
		}

		// サイズ設定を取得
		const SpeechBubbleSizeConfig* sizeCfg = &sbConfig->sizeSmall;
		switch (bubble->sizeCategory) {
		case 0: sizeCfg = &sbConfig->sizeLarge;  break;
		case 1: sizeCfg = &sbConfig->sizeMedium; break;
		default: break;
		}

		const No::Vector2 maxScale = sizeCfg->spriteSize;
		const No::Vector2 minScale = sizeCfg->spriteSize * std::max(0.01f, sbConfig->initialScalePercent);

		// ────────────────────────────────────────────────
		// Appearing フェーズ: バウンスイージングで最大サイズへ拡大
		// ────────────────────────────────────────────────
		if (bubble->phase == SpeechBubblePhase::Appearing) {
			bubble->appearElapsed += deltaTime;
			const float appearDur = std::max(0.01f, sizeCfg->appearDuration);
			float rate = bubble->appearElapsed / appearDur;
			rate = std::min(rate, 1.0f);

			t->scale = LerpV2(minScale, maxScale, EaseBounceOut(rate));

			if (rate >= 1.0f) {
				t->scale = maxScale;
				bubble->phase = SpeechBubblePhase::Stopping;
			}
			continue;
		}

		// ────────────────────────────────────────────────
		// Stopping フェーズ: 最大サイズで停止
		// ────────────────────────────────────────────────
		if (bubble->phase == SpeechBubblePhase::Stopping) {
			t->scale = maxScale;
			bubble->stopElapsed += deltaTime;
			if (bubble->stopElapsed >= sizeCfg->stopDuration) {
				bubble->phase = SpeechBubblePhase::Moving;
			}
			continue;
		}

		// ────────────────────────────────────────────────
		// Moving フェーズ: ベジェ曲線でボスHPバーへ移動
		// ────────────────────────────────────────────────
		bubble->elapsed += deltaTime;
		const float duration = std::max(0.05f, bubble->duration);
		const float rate     = std::min(1.0f, bubble->elapsed / duration);
		t->translate = Bezier2(bubble->start, bubble->control, bubble->end, rate);

		if (rate < 1.0f) {
			continue;
		}

		// 到達: ボスへダメージ送信
		if (bossEntity != No::nullEntity) {
			auto req    = registry.GenerateEntity();
			auto* damage = registry.AddComponent<DamageRequestComponent>(req);
			damage->target           = bossEntity;
			damage->amount           = std::max(1, bubble->attackPower);
			damage->ignoreInvincible = true;
		}

		toDestroy.push_back(entity);
	}

	for (auto e : toDestroy) {
		registry.DestroyEntity(e);
	}
}
