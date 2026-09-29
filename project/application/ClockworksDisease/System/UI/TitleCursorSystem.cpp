#include "stdafx.h"
#include "TitleCursorSystem.h"
#include "application/ClockworksDisease/Component/UI/TitleUIComponent.h"
#include "application/ClockworksDisease/Component/Game/GameQuitEvent.h"


void TitleCursorSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	for (auto e : registry.View<No::Transform2DComponent, TitleCursorComponent>()) {
		auto* cursor = registry.GetComponent<TitleCursorComponent>(e);
		if (cursor->selected) continue;

		auto input = No::GetInputAxisValue("Vertical");
		auto preInput = No::GetInputPreAxisValue("Vertical");

		switch (cursor->options) {
		case TitleCursorComponent::Options::kStart: {
			if (No::InputIsTrigger("Down") || input < 0.0f && preInput == 0.0f) {
				auto* t = registry.GetComponent<No::Transform2DComponent>(e);
				t->translate.y += cursor->positionDuration;
				cursor->options = TitleCursorComponent::Options::kQuit;
			}
			if (No::InputIsTrigger("Choise")) {
				No::SceneChangeEvent sceneChangeEvent;
				sceneChangeEvent.nextScene = "GameScene";
				registry.EmitEvent(sceneChangeEvent);
				cursor->selected = true;
			}
			break;
		}
		case TitleCursorComponent::Options::kQuit: {
			if (No::InputIsTrigger("Up") || input > 0.0f && preInput == 0.0f) {
				auto* t = registry.GetComponent<No::Transform2DComponent>(e);
				t->translate.y -= cursor->positionDuration;
				cursor->options = TitleCursorComponent::Options::kStart;
			}
			if (No::InputIsTrigger("Choise")) {
				cursor->selected = true;
				registry.EmitEvent(GameQuitEvent());

			}
			break;
		}
		default:
			break;
		}
	}

}
