#include "stdafx.h"
#include "OptionSystem.h"
#include "application/CommentBout/Component/OutGame/OptionStateComponent.h"
#include "application/CommentBout/Component/OutGame/OptionMenuConfigComponent.h"
#include "application/CommentBout/Event/OptionMenuEvent.h"
#include "application/CommentBout/Utility/CBGameAudio.h"
#include "application/CommentBout/Utility/InputHelper.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/GameTag.h"
#include <algorithm>

namespace {
	float SafeDuration(float d) {
		return (d <= 0.0001f) ? 0.0001f : d;
	}

	void StartOptionPhase(OptionStateComponent* state, int phase, float duration) {
		state->phase = phase;
		state->phaseTime = 0.0f;
		state->phaseDuration = SafeDuration(duration);
	}
}

void OptionSystem::Update(No::Registry& registry, float deltaTime)
{
	OptionStateComponent* optionState = nullptr;
	OptionMenuConfigComponent* optionConfig = nullptr;

	auto stateView = registry.View<CBOptionStateTag, OptionStateComponent>();
	for (auto entity : stateView) {
		optionState = registry.GetComponent<OptionStateComponent>(entity);
		if (optionState) {
			break;
		}
	}
	auto configView = registry.View<CBOptionConfigTag, OptionMenuConfigComponent>();
	for (auto entity : configView) {
		optionConfig = registry.GetComponent<OptionMenuConfigComponent>(entity);
		if (optionConfig) {
			break;
		}
	}
	if (!optionState || !optionConfig) {
		return;
	}

	while (const auto openEvent = registry.PollEvent<CommentBout::OpenOptionMenuEvent>()) {
		if (optionState->phase == OptionStateComponent::Closed) {
			optionState->isOpen = true;
			optionState->isEditing = false;
			optionState->selectedIndex = 0;
			optionState->owner = openEvent->owner;
			optionState->isConfirmAnimating = false;
			optionState->confirmIndex = -1;
			optionState->confirmAnimTime = 0.0f;
			optionState->requestedAction = OptionStateComponent::None;
			StartOptionPhase(optionState, OptionStateComponent::Opening, optionConfig->openDuration);
		}
	}

	CommentBout::GameAudio::ApplyOptionVolumes(
		optionState->masterVolume,
		optionState->bgmVolume,
		optionState->seVolume
	);

	if (optionState->phase == OptionStateComponent::Closed) {
		optionState->isOpen = false;
		optionState->isEditing = false;
		optionState->isConfirmAnimating = false;
		optionState->confirmIndex = -1;
		optionState->confirmAnimTime = 0.0f;
		optionState->requestedAction = OptionStateComponent::None;
		return;
	}

	if (optionState->phase == OptionStateComponent::Opening || optionState->phase == OptionStateComponent::Closing) {
		optionState->phaseTime += deltaTime;
		if (optionState->phaseTime >= optionState->phaseDuration) {
			if (optionState->phase == OptionStateComponent::Opening) {
				StartOptionPhase(optionState, OptionStateComponent::OpenSelect, 1.0f);
			}
			else {
				StartOptionPhase(optionState, OptionStateComponent::Closed, 1.0f);
				optionState->isOpen = false;
				optionState->isEditing = false;
				CommentBout::OptionMenuClosedEvent event;
				event.owner = optionState->owner;
				registry.EmitEvent(event);
				optionState->owner = CommentBout::OptionMenuOwner::Unknown;
			}
		}
		return;
	}

	if (optionState->isConfirmAnimating) {
		optionState->confirmAnimTime += deltaTime;
		const float confirmDuration = SafeDuration(optionConfig->confirmDuration);
		if (optionState->confirmAnimTime >= confirmDuration) {
			optionState->isConfirmAnimating = false;
			optionState->confirmAnimTime = 0.0f;
			optionState->confirmIndex = -1;

			if (optionState->requestedAction == OptionStateComponent::CloseOption) {
				StartOptionPhase(optionState, OptionStateComponent::Closing, optionConfig->closeDuration);
			}
			else if (optionState->requestedAction == OptionStateComponent::StartEdit) {
				optionState->isEditing = true;
				StartOptionPhase(optionState, OptionStateComponent::OpenEdit, 1.0f);
			}
			optionState->requestedAction = OptionStateComponent::None;
		}
		return;
	}

	if (optionState->phase == OptionStateComponent::OpenEdit) {
		const float step = std::max(0.01f, optionConfig->volumeStep);
		const bool dec = InputHelper::IsMoveLeftTrigger();
		const bool inc = InputHelper::IsMoveRightTrigger();
		const float beforeMaster = optionState->masterVolume;
		const float beforeBGM = optionState->bgmVolume;
		const float beforeSE = optionState->seVolume;

		if (optionState->selectedIndex == 0) {
			if (dec) optionState->masterVolume = std::max(0.0f, optionState->masterVolume - step);
			if (inc) optionState->masterVolume = std::min(1.0f, optionState->masterVolume + step);
		}
		else if (optionState->selectedIndex == 1) {
			if (dec) optionState->bgmVolume = std::max(0.0f, optionState->bgmVolume - step);
			if (inc) optionState->bgmVolume = std::min(1.0f, optionState->bgmVolume + step);
		}
		else if (optionState->selectedIndex == 2) {
			if (dec) optionState->seVolume = std::max(0.0f, optionState->seVolume - step);
			if (inc) optionState->seVolume = std::min(1.0f, optionState->seVolume + step);
		}
		else if (optionState->selectedIndex == 3) {
			if (dec || inc) {
				optionState->vibrationEnabled = !optionState->vibrationEnabled;
			}
		}

		const bool volumeChanged =
			(beforeMaster != optionState->masterVolume) ||
			(beforeBGM != optionState->bgmVolume) ||
			(beforeSE != optionState->seVolume);
		if (volumeChanged) {
			CommentBout::GameAudio::PlaySEClip(CommentBoutResourceKey::kSESystemMoveCursor);
		}

		if (InputHelper::IsConfirmTrigger()) {
			optionState->isEditing = false;
			StartOptionPhase(optionState, OptionStateComponent::OpenSelect, 1.0f);
		}
		return;
	}

	if (InputHelper::IsMoveUpTrigger()) {
		optionState->selectedIndex--;
		if (optionState->selectedIndex < 0) {
			optionState->selectedIndex = optionState->itemCount - 1;
		}
		CommentBout::GameAudio::PlaySEClip(CommentBoutResourceKey::kSESystemMoveCursor);
	}
	if (InputHelper::IsMoveDownTrigger()) {
		optionState->selectedIndex++;
		if (optionState->selectedIndex >= optionState->itemCount) {
			optionState->selectedIndex = 0;
		}
		CommentBout::GameAudio::PlaySEClip(CommentBoutResourceKey::kSESystemMoveCursor);
	}

	if (InputHelper::IsConfirmTrigger()) {
		CommentBout::GameAudio::PlaySEClip(CommentBoutResourceKey::kSESystemDecision);
		optionState->isConfirmAnimating = true;
		optionState->confirmIndex = optionState->selectedIndex;
		optionState->confirmAnimTime = 0.0f;
		optionState->requestedAction = (optionState->selectedIndex == 4)
			? OptionStateComponent::CloseOption
			: OptionStateComponent::StartEdit;
	}
}
