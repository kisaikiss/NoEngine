#include "stdafx.h"
#include "PauseSystem.h"
#include "application/CommentBout/Component/OutGame/PauseStateComponent.h"
#include "application/CommentBout/Component/OutGame/PauseMenuConfigComponent.h"
#include "application/CommentBout/Component/OutGame/OptionStateComponent.h"
#include "application/CommentBout/Event/OptionMenuEvent.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Utility/InputHelper.h"
#include "application/CommentBout/Utility/CBGameAudio.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "engine/Functions/ECS/Component/PauseComponent.h"

namespace {
	float SafeDuration(float d) {
		return (d <= 0.0001f) ? 0.0001f : d;
	}

	void StartPhase(PauseStateComponent* state, int phase, float duration) {
		state->phase = phase;
		state->phaseTime = 0.0f;
		state->phaseDuration = SafeDuration(duration);
	}

	No::PauseComponent* FindEnginePauseComponent(No::Registry& registry) {
		auto pauseView = registry.View<No::PauseComponent>();
		auto it = pauseView.begin();
		if (it == pauseView.end()) {
			return nullptr;
		}
		return registry.GetComponent<No::PauseComponent>(*it);
	}

	void SyncPauseState(PauseStateComponent* pauseState, No::PauseComponent* enginePause, bool forcePause) {
		const bool isPaused = forcePause || (pauseState->phase != PauseStateComponent::Closed);
		pauseState->isPaused = isPaused;
		if (enginePause) {
			enginePause->isPause = isPaused;
		}
	}

	void ClearPauseState(PauseStateComponent* pauseState, No::PauseComponent* enginePause) {
		pauseState->isPaused = false;
		pauseState->phase = PauseStateComponent::Closed;
		pauseState->phaseTime = 0.0f;
		pauseState->phaseDuration = 1.0f;
		pauseState->isConfirmAnimating = false;
		pauseState->confirmAnimTime = 0.0f;
		pauseState->confirmIndex = -1;
		pauseState->requestedAction = PauseStateComponent::None;
		if (enginePause) {
			enginePause->isPause = false;
		}
	}
}

void PauseSystem::Update(No::Registry& registry, float deltaTime)
{
	PauseStateComponent* pauseState = nullptr;
	PauseMenuConfigComponent* pauseConfig = nullptr;
	OptionStateComponent* optionState = nullptr;
	No::PauseComponent* enginePause = FindEnginePauseComponent(registry);

	auto pauseView = registry.View<CBPauseStateTag, PauseStateComponent>();
	for (auto entity : pauseView) {
		pauseState = registry.GetComponent<PauseStateComponent>(entity);
		if (pauseState) {
			break;
		}
	}

	auto configView = registry.View<CBPauseConfigTag, PauseMenuConfigComponent>();
	for (auto entity : configView) {
		pauseConfig = registry.GetComponent<PauseMenuConfigComponent>(entity);
		if (pauseConfig) {
			break;
		}
	}

	auto optionStateView = registry.View<CBOptionStateTag, OptionStateComponent>();
	for (auto entity : optionStateView) {
		optionState = registry.GetComponent<OptionStateComponent>(entity);
		if (optionState) {
			break;
		}
	}

	if (!pauseState || !pauseConfig) {
		if (enginePause) {
			enginePause->isPause = false;
		}
		return;
	}

	const bool forcePause = pauseState->editorForcePause;

	pauseState->justEnteredPause = false;
	pauseState->justExitedPause = false;

	if (pauseState->isConfirmAnimating) {
		pauseState->confirmAnimTime += deltaTime;
		const float confirmDuration = SafeDuration(pauseConfig->confirmDuration);
		if (pauseState->confirmAnimTime >= confirmDuration) {
			pauseState->isConfirmAnimating = false;
			pauseState->confirmAnimTime = 0.0f;
			pauseState->confirmIndex = -1;

			switch (pauseState->requestedAction) {
			case PauseStateComponent::Resume:
				StartPhase(pauseState, PauseStateComponent::Closing, pauseConfig->closeDuration);
				break;
			case PauseStateComponent::Restart:
			{
				// シーン遷移前にポーズ状態を明示解除して、停止状態の持ち越しを防ぐ。
				ClearPauseState(pauseState, enginePause);
				No::SceneChangeEvent event;
				event.nextScene = "GameScene";
				registry.EmitEvent(event);
			}
			break;
			case PauseStateComponent::OpenOption:
			{
				CommentBout::OpenOptionMenuEvent event;
				event.owner = CommentBout::OptionMenuOwner::Pause;
				registry.EmitEvent(event);
			}
			break;
			case PauseStateComponent::BackToTitle:
			{
				// シーン遷移前にポーズ状態を明示解除して、停止状態の持ち越しを防ぐ。
				ClearPauseState(pauseState, enginePause);
				No::SceneChangeEvent event;
				event.nextScene = "TitleScene";
				registry.EmitEvent(event);
			}
			break;
			default:
				break;
			}
			pauseState->requestedAction = PauseStateComponent::None;
		}
	}

	if (pauseState->phase == PauseStateComponent::Closed) {
		pauseState->isPaused = false;
		if (InputHelper::IsPauseTrigger()) {
			CommentBout::GameAudio::PlaySEClip(CommentBoutResourceKey::kSESystemOpen);
			pauseState->justEnteredPause = true;
			pauseState->selectedIndex = 0;
			pauseState->requestedAction = PauseStateComponent::None;
			pauseState->isConfirmAnimating = false;
			pauseState->confirmAnimTime = 0.0f;
			pauseState->confirmIndex = -1;
			StartPhase(pauseState, PauseStateComponent::Opening, pauseConfig->openDuration);
		}
		SyncPauseState(pauseState, enginePause, forcePause);
		return;
	}

	pauseState->isPaused = true;

	if (pauseState->phase == PauseStateComponent::Opening ||
		pauseState->phase == PauseStateComponent::Closing) {
		pauseState->phaseTime += deltaTime;
		if (pauseState->phaseTime >= pauseState->phaseDuration) {
			switch (pauseState->phase) {
			case PauseStateComponent::Opening:
				StartPhase(pauseState, PauseStateComponent::Open, 1.0f);
				pauseState->phaseTime = 0.0f;
				break;
			case PauseStateComponent::Closing:
				StartPhase(pauseState, PauseStateComponent::Closed, 1.0f);
				pauseState->isPaused = false;
				pauseState->justExitedPause = true;
				break;
			default:
				break;
			}
		}
		SyncPauseState(pauseState, enginePause, forcePause);
		return;
	}

	if (optionState && optionState->isOpen) {
		SyncPauseState(pauseState, enginePause, forcePause);
		return;
	}

	if (pauseState->isConfirmAnimating) {
		SyncPauseState(pauseState, enginePause, forcePause);
		return;
	}

	if (pauseState->phase != PauseStateComponent::Open) {
		SyncPauseState(pauseState, enginePause, forcePause);
		return;
	}

	if (pauseState->itemCount > 0) {
		if (InputHelper::IsMoveUpTrigger()) {
			pauseState->selectedIndex--;
			if (pauseState->selectedIndex < 0) {
				pauseState->selectedIndex = pauseState->itemCount - 1;
			}
			CommentBout::GameAudio::PlaySEClip(CommentBoutResourceKey::kSESystemMoveCursor);
		}
		if (InputHelper::IsMoveDownTrigger()) {
			pauseState->selectedIndex++;
			if (pauseState->selectedIndex >= pauseState->itemCount) {
				pauseState->selectedIndex = 0;
			}
			CommentBout::GameAudio::PlaySEClip(CommentBoutResourceKey::kSESystemMoveCursor);
		}
	}

	if (InputHelper::IsConfirmTrigger()) {
		int action = PauseStateComponent::None;
		switch (pauseState->selectedIndex) {
		case 0:
			action = PauseStateComponent::Resume;
			break;
		case 1:
			action = PauseStateComponent::Restart;
			break;
		case 2:
			action = PauseStateComponent::OpenOption;
			break;
		case 3:
			action = PauseStateComponent::BackToTitle;
			break;
		default:
			action = PauseStateComponent::None;
			break;
		}

		if (action != PauseStateComponent::None) {
			CommentBout::GameAudio::PlaySEClip(CommentBoutResourceKey::kSESystemDecision);
			pauseState->confirmIndex = pauseState->selectedIndex;
			pauseState->confirmAnimTime = 0.0f;
			pauseState->isConfirmAnimating = true;
			pauseState->requestedAction = action;
		}
	}

	SyncPauseState(pauseState, enginePause, forcePause);
}
