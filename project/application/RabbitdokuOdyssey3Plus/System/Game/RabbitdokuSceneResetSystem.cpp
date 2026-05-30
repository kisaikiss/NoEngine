#include "stdafx.h"
#include "RabbitdokuSceneResetSystem.h"
#include "../../Game/RabbitdokuResetEvent.h"
#include "../../Game/RabbitdokuSerializer.h"

void RabbitdokuSceneResetSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	

	auto event = registry.PollEvent<RabbitdokuResetEvent>();
	if (event.has_value()) {
		RabbitdokuSerializer::GameSave(registry, event->saveData.respawnPoint, event->saveData.death, event->saveData.totalDeath);
		No::SceneChangeEvent sceneChange;
		sceneChange.nextScene = event->nextScene;
		registry.EmitEvent(sceneChange);
	}

}
