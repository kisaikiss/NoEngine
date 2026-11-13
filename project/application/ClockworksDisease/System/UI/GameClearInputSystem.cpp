#include "GameClearInputSystem.h"

void GameClearInputSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	if (No::InputIsTrigger("Choise")) {
		No::SceneChangeEvent event;
		event.nextScene = "TitleScene";
		registry.EmitEvent(event);
	}
}

REGISTER_SYSTEM(::GameClearInputSystem, "GameClearInputSystem", "UI")
