#include "stdafx.h"
#include "RabbitdokuSceneResetSystem.h"
#include "../../Game/RabbitdokuResetEvent.h"

void RabbitdokuSceneResetSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	if (No::InputIsTrigger("Reset")) {
		RabbitdokuResetEvent dead;
		registry.EmitEvent(dead);
	}

	auto event = registry.PollEvent<RabbitdokuResetEvent>();
	if (event.has_value()) {
		No::SceneChangeEvent sceneChange;
		sceneChange.nextScene = No::GetCurrentSceneName(registry);
		registry.EmitEvent(sceneChange);
	}

}
