#include "stdafx.h"
#include "TitleSystem.h"
#include "application/CommentBout/Component/OutGame/TitleMenuStateComponent.h"
#include "application/CommentBout/Component/OutGame/TitleMenuConfigComponent.h"
#include "application/CommentBout/Component/OutGame/OptionStateComponent.h"
#include "application/CommentBout/Event/OptionMenuEvent.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Utility/InputHelper.h"
#include "application/CommentBout/Utility/CBGameAudio.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "engine/Runtime/GraphicsCore.h"

namespace {
	float SafeDuration(float d) {
		return (d <= 0.0001f) ? 0.0001f : d;
	}
}

void TitleSystem::Update(No::Registry& registry, float deltaTime)
{
	TitleMenuStateComponent* titleState = nullptr;
	TitleMenuConfigComponent* titleConfig = nullptr;
	OptionStateComponent* optionState = nullptr;

	auto stateView = registry.View<CBTitleStateTag, TitleMenuStateComponent>();
	for (auto entity : stateView) {
		titleState = registry.GetComponent<TitleMenuStateComponent>(entity);
		if (titleState) {
			break;
		}
	}

	auto configView = registry.View<CBTitleConfigTag, TitleMenuConfigComponent>();
	for (auto entity : configView) {
		titleConfig = registry.GetComponent<TitleMenuConfigComponent>(entity);
		if (titleConfig) {
			break;
		}
	}

	auto optionView = registry.View<CBOptionStateTag, OptionStateComponent>();
	for (auto entity : optionView) {
		optionState = registry.GetComponent<OptionStateComponent>(entity);
		if (optionState) {
			break;
		}
	}

	if (!titleState || !titleConfig) {
		return;
	}

	titleState->logoMotionTime += deltaTime;

	// エンジンのシーン遷移フェードイン中は入力を受け付けない
	if (entryBlockTimer_ > 0.0f) {
		entryBlockTimer_ -= deltaTime;
		return;
	}

	if (titleState->isConfirmAnimating) {
		titleState->confirmAnimTime += deltaTime;
		if (titleState->confirmAnimTime >= SafeDuration(titleConfig->confirmDuration)) {
			titleState->isConfirmAnimating = false;
			titleState->confirmAnimTime = 0.0f;
			titleState->confirmIndex = -1;

			switch (titleState->requestedAction) {
			case TitleMenuStateComponent::StartGame:
			{
				titleState->isSceneChangePending = true;
				No::SceneChangeEvent event;
				event.nextScene = "GameScene";
				registry.EmitEvent(event);
			}
			break;
			case TitleMenuStateComponent::OpenOption:
			{
				CommentBout::OpenOptionMenuEvent event;
				event.owner = CommentBout::OptionMenuOwner::Title;
				registry.EmitEvent(event);
			}
			break;
			case TitleMenuStateComponent::ExitApp:
			{
				auto* mainWindow = NoEngine::GraphicsCore::gWindowManager.GetMainWindow();
				if (mainWindow) {
					mainWindow->Destroy();
				}
			}
			break;
			default:
				break;
			}

			titleState->requestedAction = TitleMenuStateComponent::None;
		}
	}

	if ((optionState && optionState->isOpen) || titleState->isConfirmAnimating || titleState->isSceneChangePending) {
		return;
	}

	if (titleState->itemCount > 0) {
		if (InputHelper::IsMoveUpTrigger()) {
			titleState->selectedIndex--;
			if (titleState->selectedIndex < 0) {
				titleState->selectedIndex = titleState->itemCount - 1;
			}
			CommentBout::GameAudio::PlaySEClip(CommentBoutResourceKey::kSESystemMoveCursor);
		}
		if (InputHelper::IsMoveDownTrigger()) {
			titleState->selectedIndex++;
			if (titleState->selectedIndex >= titleState->itemCount) {
				titleState->selectedIndex = 0;
			}
			CommentBout::GameAudio::PlaySEClip(CommentBoutResourceKey::kSESystemMoveCursor);
		}
	}

	if (InputHelper::IsConfirmTrigger()) {
		int action = TitleMenuStateComponent::None;
		switch (titleState->selectedIndex) {
		case 0:
			action = TitleMenuStateComponent::StartGame;
			break;
		case 1:
			action = TitleMenuStateComponent::OpenOption;
			break;
		case 2:
			action = TitleMenuStateComponent::ExitApp;
			break;
		default:
			break;
		}

		if (action != TitleMenuStateComponent::None) {
			CommentBout::GameAudio::PlaySEClip(CommentBoutResourceKey::kSESystemDecision);
			titleState->confirmIndex = titleState->selectedIndex;
			titleState->confirmAnimTime = 0.0f;
			titleState->isConfirmAnimating = true;
			titleState->requestedAction = action;
		}
	}
}
