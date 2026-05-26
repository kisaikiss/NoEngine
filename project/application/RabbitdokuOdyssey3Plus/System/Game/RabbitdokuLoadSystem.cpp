#include "stdafx.h"
#include "RabbitdokuLoadSystem.h"
#include "application/RabbitdokuOdyssey3Plus/Component/RabbitdokuComponent.h"
#include "application/RabbitdokuOdyssey3Plus/Game/RabbitdokuSerializer.h"
#include "../../Component/FollowCamera2DComponent.h"

void RabbitdokuLoadSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	if (isStartFrame_) {
		SaveData* saveData = nullptr;

		auto view = registry.View<Rabbitdoku, SaveData, No::Transform2DComponent>();
		for (auto e : view) {
			saveData = registry.GetComponent<SaveData>(e);
			auto loadData = RabbitdokuSerializer::GameLoad(registry);
			saveData->respawnPoint = loadData.respawnPoint;
			saveData->death = loadData.death;
			saveData->totalDeath = loadData.totalDeath;

			if (saveData->respawnPoint == No::Vector2::ZERO) continue;
			
			auto* t = registry.GetComponent<No::Transform2DComponent>(e);
			t->translate = saveData->respawnPoint;
		}

		if (!saveData) return;

		auto cameraView = registry.View<FollowCamera2DComponent, No::Transform2DComponent>();
		for (auto e : cameraView) {
			auto* t = registry.GetComponent<No::Transform2DComponent>(e);
			t->translate = saveData->respawnPoint;
		}

		isStartFrame_ = false;
	}
}
