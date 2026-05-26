#include "stdafx.h"
#include "RabbitdokuSceneResetSystem.h"
#include "../../Game/RabbitdokuResetEvent.h"
#include "../../Game/RabbitdokuSerializer.h"

void RabbitdokuSceneResetSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	

	auto event = registry.PollEvent<RabbitdokuResetEvent>();
	if (event.has_value()) {
		auto view = registry.View<SaveData>();

		SaveData saveData;

		for (auto e : view) {
			saveData = *registry.GetComponent<SaveData>(e);
		}

		RabbitdokuSerializer::GameSave(registry, saveData.respawnPoint, saveData.death, saveData.totalDeath);
		No::SceneChangeEvent sceneChange;
		sceneChange.nextScene = No::GetCurrentSceneName(registry);
		registry.EmitEvent(sceneChange);
	}

}
