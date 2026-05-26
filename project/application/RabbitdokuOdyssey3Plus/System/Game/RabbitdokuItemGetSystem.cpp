#include "stdafx.h"
#include "RabbitdokuItemGetSystem.h"
#include "../../Component/RabbitdokuComponent.h"
#include "../../Component/SaveDataComponent.h"
#include "../../Game/RabbitdokuCollisionLayer.h"
#include "../../Game/RabbitdokuTag.h"
#include "application/RabbitdokuOdyssey3Plus/Game/RabbitdokuSerializer.h"


void RabbitItemGetSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto view = registry.PollAllEvents<RabbitdokuItemGetEvent>();

	for (auto event : view) {
		//auto* player = registry.GetComponent<Rabbitdoku>(event.player);
		if (registry.GetComponent<SaveTag>(event.item)) {
			if (No::InputIsTrigger("Save")) {
				RabbitdokuSerializer::GameSave(registry, registry.GetComponent<No::Transform2DComponent>(event.player)->translate);
			}
		}

	}

}
