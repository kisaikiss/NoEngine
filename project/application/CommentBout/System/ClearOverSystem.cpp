#include "stdafx.h"
#include "ClearOverSystem.h"
#include "application/CommentBout/Component/ClearOverStateComponent.h"
#include "application/CommentBout/Component/ClearOverConfigComponent.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Utility/InputHelper.h"
#include "application/CommentBout/Utility/CBGameAudio.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "engine/Functions/ECS/Event/SceneChangeEvent.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4702)
#endif

namespace {
float SafeDuration(float d) {
	return (d <= 0.0001f) ? 0.0001f : d;
}
}

void ClearOverSystem::Update(No::Registry& registry, float deltaTime)
{
	ClearOverStateComponent* state = nullptr;
	ClearOverConfigComponent* config = nullptr;

	auto stateView = registry.View<CBClearOverStateTag, ClearOverStateComponent>();
	auto configView = registry.View<CBClearOverConfigTag, ClearOverConfigComponent>();

	for (auto e : stateView) {
		state = registry.GetComponent<ClearOverStateComponent>(e);
		break;
	}
	for (auto e : configView) {
		config = registry.GetComponent<ClearOverConfigComponent>(e);
		break;
	}

	if (!state || !config || state->phase == ClearOverStateComponent::Phase::Inactive) {
		return;
	}

	// ── 確認アニメーション処理 ────────────────────────────────
	if (state->isConfirmAnimating) {
		state->confirmAnimTime += deltaTime;
		if (state->confirmAnimTime >= SafeDuration(config->confirmDuration)) {
			state->isConfirmAnimating = false;
			state->confirmAnimTime = 0.f;
			state->confirmIndex = -1;

			const auto action = state->requestedAction;
			state->requestedAction = ClearOverStateComponent::Action::None;
			state->phase = ClearOverStateComponent::Phase::Inactive;

			No::SceneChangeEvent event;
			switch (action) {
			case ClearOverStateComponent::Action::Restart:
				event.nextScene = "GameScene";
				registry.EmitEvent(event);
				break;
			case ClearOverStateComponent::Action::BackToTitle:
				event.nextScene = "TitleScene";
				registry.EmitEvent(event);
				break;
			default:
				break;
			}
		}
		return;
	}

	state->phaseTimer += deltaTime;

	switch (state->phase) {
	case ClearOverStateComponent::Phase::FadeIn:
		if (state->phaseTimer >= SafeDuration(config->fadeDuration)) {
			state->phase = ClearOverStateComponent::Phase::LogoAppear;
			state->phaseTimer = 0.f;
		}
		break;

	case ClearOverStateComponent::Phase::LogoAppear:
		if (state->phaseTimer >= SafeDuration(config->logoAppearDuration)) {
			CommentBout::GameAudio::PlaySEClip(CommentBoutResourceKey::kSESystemOpen);
			state->phase = ClearOverStateComponent::Phase::MenuAppear;
			state->phaseTimer = 0.f;
		}
		break;

	case ClearOverStateComponent::Phase::MenuAppear:
		if (state->phaseTimer >= SafeDuration(config->menuAppearDuration)) {
			state->phase = ClearOverStateComponent::Phase::MenuActive;
			state->phaseTimer = 0.f;
		}
		break;

	case ClearOverStateComponent::Phase::MenuActive:
	{
		if (InputHelper::IsMoveUpTrigger() && state->selectedIndex > 0) {
			state->selectedIndex--;
			CommentBout::GameAudio::PlaySEClip(CommentBoutResourceKey::kSESystemMoveCursor);
		}
		if (InputHelper::IsMoveDownTrigger() && state->selectedIndex < 1) {
			state->selectedIndex++;
			CommentBout::GameAudio::PlaySEClip(CommentBoutResourceKey::kSESystemMoveCursor);
		}

		if (InputHelper::IsConfirmTrigger()) {
			CommentBout::GameAudio::PlaySEClip(CommentBoutResourceKey::kSESystemDecision);
			state->confirmIndex = state->selectedIndex;
			state->confirmAnimTime = 0.f;
			state->isConfirmAnimating = true;
			state->requestedAction = (state->selectedIndex == 0)
				? ClearOverStateComponent::Action::Restart
				: ClearOverStateComponent::Action::BackToTitle;
		}
		break;
	}
	default:
		break;
	}
}
