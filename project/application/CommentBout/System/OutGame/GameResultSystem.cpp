#include "stdafx.h"
#include "GameResultSystem.h"
#include "application/CommentBout/Component/OutGame/GameResultComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Component/Enemy/BossDefeatSequenceComponent.h"
#include "application/CommentBout/Component/OutGame/ClearOverStateComponent.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Utility/CBGameAudio.h"
#include "application/CommentBout/Component/GameResourceComponent.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4702)
#endif

namespace {
void ActivateClearOver(No::Registry& registry, ClearOverStateComponent::Result result) {
	auto view = registry.View<CBClearOverStateTag, ClearOverStateComponent>();
	for (auto e : view) {
		auto* state = registry.GetComponent<ClearOverStateComponent>(e);
		if (state && state->phase == ClearOverStateComponent::Phase::Inactive) {
			CommentBout::GameAudio::StopBGMClip(CommentBoutResourceKey::kBGMInGame);
			state->result = result;
			state->phase = ClearOverStateComponent::Phase::FadeIn;
			state->phaseTimer = 0.f;
			state->selectedIndex = 0;
			state->isConfirmAnimating = false;
			state->confirmAnimTime = 0.f;
			state->confirmIndex = -1;
			state->requestedAction = ClearOverStateComponent::Action::None;
		}
		break;
	}
}

}

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

	// ── クリア判定: ボスが死亡 ──────────────────────────────
	if (!gameResult->debugDisableClear) {
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
	if (!gameResult->debugDisableOver) {
		auto playerView = registry.View<CBPlayerTag, HealthComponent>();
		for (auto playerEntity : playerView) {
			auto* health = registry.GetComponent<HealthComponent>(playerEntity);
			if (health && health->isDead) {
				gameResult->result = GameResult::Over;
				gameResult->playerDied = true;
				ActivateClearOver(registry, ClearOverStateComponent::Result::Over);
				return;
			}
		}

		// ── オーバー判定: レール終端 ──────────────────────────
		if (gameResult->railReachedEnd) {
			gameResult->result = GameResult::Over;
			ActivateClearOver(registry, ClearOverStateComponent::Result::Over);
		}
	}

	DebugToggleClearOver(gameResult);
}

void GameResultSystem::DebugToggleClearOver(GameResultComponent* gameResult)
{

#ifdef USE_IMGUI

	// デバッグ用無敵トグル（左右Shift同時押し）
	const bool shiftToggle =
		(No::Keyboard::IsTrigger(VK_LSHIFT) && No::Keyboard::IsPress(VK_RSHIFT)) ||
		(No::Keyboard::IsTrigger(VK_RSHIFT) && No::Keyboard::IsPress(VK_LSHIFT));
	if (shiftToggle) {
		gameResult->debugDisableOver = !gameResult->debugDisableOver;
		gameResult->debugDisableClear = !gameResult->debugDisableClear;
	}

	if (gameResult->debugDisableOver || gameResult->debugDisableClear) {
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		if (viewport) {
			ImGui::SetNextWindowPos(
				ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 16.0f, viewport->WorkPos.y + 32.0f),
				ImGuiCond_Always,
				ImVec2(1.0f, 0.0f)
			);
		}
		ImGui::SetNextWindowBgAlpha(0.35f);
		ImGui::Begin("PlayerInvincibleOverlay", nullptr,
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav);
		if (gameResult->debugDisableClear && gameResult->debugDisableOver) {
			ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "クリアオーバー無効");
		} else if (gameResult->debugDisableClear) {
			ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "クリア無効");
		} else {
			ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "オーバー無効");
		}


		ImGui::End();
	}
#else
	static_cast<void>(gameResult);
#endif
}
