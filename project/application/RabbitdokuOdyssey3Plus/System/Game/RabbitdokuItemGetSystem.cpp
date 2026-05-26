#include "stdafx.h"
#include "RabbitdokuItemGetSystem.h"
#include "../../Component/RabbitdokuComponent.h"
#include "../../Game/RabbitdokuCollisionLayer.h"
#include "../../Game/RabbitdokuTag.h"
#include "application/RabbitdokuOdyssey3Plus/Game/RabbitdokuSerializer.h"
#include "../../Game/RabbitdokuResetEvent.h"

void RabbitItemGetSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto view = registry.PollAllEvents<RabbitdokuItemGetEvent>();

	for (auto event : view) {
		//auto* player = registry.GetComponent<Rabbitdoku>(event.player);
		if (registry.GetComponent<SaveTag>(event.item)) {
			if (No::InputIsTrigger("Save")) {
				auto* data = registry.GetComponent<SaveData>(event.player);
				RabbitdokuSerializer::GameSave(registry,
					registry.GetComponent<No::Transform2DComponent>(event.player)->translate,
					data->death, data->totalDeath);
				data->respawnPoint = registry.GetComponent<No::Transform2DComponent>(event.player)->translate;
			}
		}

		if (registry.GetComponent<EnemyTag>(event.item)) {
			registry.GetComponent<SaveData>(event.player)->death++;
			registry.GetComponent<SaveData>(event.player)->totalDeath++;
			RabbitdokuResetEvent dead;
			registry.EmitEvent(dead);
		}

	}

}
