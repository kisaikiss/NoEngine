#include "stdafx.h"
#include "RabbitdokuLoadSystem.h"
#include "application/RabbitdokuOdyssey3Plus/Component/RabbitdokuComponent.h"
#include "application/RabbitdokuOdyssey3Plus/Component/SaveDataComponent.h"
#include "application/RabbitdokuOdyssey3Plus/Game/RabbitdokuSerializer.h"

void RabbitdokuLoadSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	if (isStartFrame_) {
		auto view = registry.View<Rabbitdoku, SaveDataComponent, No::Transform2DComponent>();
		for (auto e : view) {
			No::Vector2 respawnPoint = RabbitdokuSerializer::GameLoad(registry);
			if (respawnPoint == No::Vector2::ZERO) continue;
				auto* t = registry.GetComponent<No::Transform2DComponent>(e);
				t->translate = respawnPoint;
		}


		isStartFrame_ = false;
	}
}
