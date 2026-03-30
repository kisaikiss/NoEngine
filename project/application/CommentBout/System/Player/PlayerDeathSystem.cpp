#include "stdafx.h"
#include "PlayerDeathSystem.h"
#include "application/CommentBout/Component/Player/PlayerAnimStateComponent.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/Component/OutGame/ClearOverStateComponent.h"
#include "application/CommentBout/Data/PlayerDeathConfig.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Utility/CBGameAudio.h"
#include "engine/Functions/ECS/Component/Transform2DComponent.h"
#include "engine/Functions/ECS/Component/PauseComponent.h"
#include "engine/Runtime/GraphicsCore.h"
#include <algorithm>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4702)
#endif

void PlayerDeathSystem::Update(No::Registry& registry, float deltaTime)
{
	// PlayerAnimState::Dead のプレイヤーエンティティを検索
	No::Entity playerEntity = No::nullEntity;
	No::Transform2DComponent* playerTransform = nullptr;

	auto playerView = registry.View<CBPlayerTag, PlayerAnimStateComponent, No::Transform2DComponent>();
	for (auto e : playerView) {
		auto* animState = registry.GetComponent<PlayerAnimStateComponent>(e);
		if (animState && animState->state == PlayerAnimState::Dead) {
			playerEntity    = e;
			playerTransform = registry.GetComponent<No::Transform2DComponent>(e);
			break;
		}
	}

	if (playerEntity == No::nullEntity) {
		// Dead になったことがなければリセット（シーン再利用対応）
		cutsceneStarted_ = false;
		return;
	}

	// ── カットシーン開始: 最初のフレームのみ ─────────────────────────
	if (!cutsceneStarted_) {
		cutsceneStarted_ = true;

		for (auto e : registry.View<CBGameResourceTag, GameResourceComponent>()) {
			auto* res = registry.GetComponent<GameResourceComponent>(e);
			if (res) { res->inCutscene = true; }
			break;
		}
		for (auto e : registry.View<No::PauseComponent>()) {
			auto* pause = registry.GetComponent<No::PauseComponent>(e);
			if (pause) { pause->isPause = true; }
			break;
		}
		CommentBout::GameAudio::StopBGMClip(CommentBoutResourceKey::kBGMInGame);
	}

	if (!playerTransform) {
		return;
	}

	// ── PlayerDeathConfig を取得 ──────────────────────────────────────
	PlayerDeathConfig config;
	for (auto e : registry.View<CBPlayerDeathConfigTag, PlayerDeathConfig>()) {
		auto* cfg = registry.GetComponent<PlayerDeathConfig>(e);
		if (cfg) { config = *cfg; }
		break;
	}

	// ── 落下移動（スクリーン座標 Y+ が下方向）──────────────────────
	playerTransform->translate.y += config.fallSpeed * deltaTime;

	// ── 画面外判定 ────────────────────────────────────────────────────
	auto* mainWindow = NoEngine::GraphicsCore::gWindowManager.GetMainWindow();
	if (!mainWindow) {
		return;
	}
	const float screenHeight = static_cast<float>(mainWindow->GetWindowSize().clientHeight);
	if (playerTransform->translate.y > screenHeight + config.offscreenOffset) {
		// ClearOverStateComponent を Over で起動
		auto clearOverView = registry.View<CBClearOverStateTag, ClearOverStateComponent>();
		for (auto e : clearOverView) {
			auto* state = registry.GetComponent<ClearOverStateComponent>(e);
			if (state && state->phase == ClearOverStateComponent::Phase::Inactive) {
				state->result = ClearOverStateComponent::Result::Over;
				state->phase  = ClearOverStateComponent::Phase::FadeIn;
				state->phaseTimer         = 0.f;
				state->selectedIndex      = 0;
				state->isConfirmAnimating = false;
				state->confirmAnimTime    = 0.f;
				state->confirmIndex       = -1;
				state->requestedAction    = ClearOverStateComponent::Action::None;
			}
			break;
		}
		cutsceneStarted_ = false;
	}
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
