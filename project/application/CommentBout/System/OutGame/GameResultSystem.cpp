#include "stdafx.h"
#include "GameResultSystem.h"
#include "application/CommentBout/Component/OutGame/GameResultComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Component/Enemy/BossDefeatSequenceComponent.h"
#include "application/CommentBout/Component/Player/PlayerAnimStateComponent.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Utility/CBGameAudio.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/Component/Editor/DebugShortcutStateComponent.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4702)
#endif

void GameResultSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);

	// GameResultComponent を取得
	GameResultComponent* gameResult = nullptr;
	auto gameResultView = registry.View<CBGameResultTag, GameResultComponent>();
	for (auto e : gameResultView) {
		gameResult = registry.GetComponent<GameResultComponent>(e);
		break;
	}
	if (!gameResult || gameResult->result != GameResult::None) {
		return;
	}

	// DebugShortcutStateComponent からデバッグ無効フラグを取得
	bool debugDisableResult = false;
	for (auto se : registry.View<DebugShortcutStateTag, DebugShortcutStateComponent>()) {
		auto* sc = registry.GetComponent<DebugShortcutStateComponent>(se);
		if (sc) { debugDisableResult = sc->debugDisableResult; }
		break;
	}

	// ── クリア判定: ボスが死亡 ──────────────────────────────
	if (!debugDisableResult) {
		auto bossView = registry.View<CBBossTag, HealthComponent>();
		for (auto bossEntity : bossView) {
			auto* health = registry.GetComponent<HealthComponent>(bossEntity);
			if (!health || (!health->isDead && health->hp > 0)) {
				continue;
			}
			gameResult->result = GameResult::Clear;

			// BossDefeatSequenceComponent を起動
			auto bossDefeatView = registry.View<CBBossDefeatTag, BossDefeatSequenceComponent>();
			for (auto e : bossDefeatView) {
				auto* defeat = registry.GetComponent<BossDefeatSequenceComponent>(e);
				if (defeat && defeat->phase == BossDefeatSequenceComponent::Phase::Inactive) {
					defeat->bossEntity = bossEntity;
					defeat->phase = BossDefeatSequenceComponent::Phase::PreExplosionWait;
					defeat->phaseTimer = 0.f;
				}
				break;
			}
			return;
		}
	}

	// ── オーバー判定: プレイヤーが死亡 ──────────────────────
	if (!debugDisableResult) {
		auto playerView = registry.View<CBPlayerTag, HealthComponent>();
		for (auto playerEntity : playerView) {
			auto* health = registry.GetComponent<HealthComponent>(playerEntity);
			if (health && health->isDead) {
				gameResult->result = GameResult::Over;
				gameResult->playerDied = true;
				// PlayerDeathSystem が落下カットシーンを管理し、完了後に ClearOverState を起動する。
				// ここでは PlayerAnimState::Dead をセットして PlayerDeathSystem に通知するだけ。
				auto* animState = registry.GetComponent<PlayerAnimStateComponent>(playerEntity);
				if (animState) {
					animState->state = PlayerAnimState::Dead;
				}
				return;
			}
		}

		// ── オーバー判定: レール終端 ──────────────────────────
		if (gameResult->railReachedEnd) {
			gameResult->result = GameResult::Over;
			gameResult->playerDied = true;
			// HP0 と同様に PlayerDeathSystem へ委譲（落下カットシーン → オーバー画面）
			for (auto playerEntity : registry.View<CBPlayerTag, PlayerAnimStateComponent>()) {
				auto* animState = registry.GetComponent<PlayerAnimStateComponent>(playerEntity);
				if (animState) { animState->state = PlayerAnimState::Dead; }
				break;
			}
		}
	}

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
