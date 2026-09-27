#include "stdafx.h"
#include "SavePointSystem.h"
#include "../../Component/Item/ItemComponent.h"

#include "../../Component/Player/PlayerComponent.h"

void SavePointSystem::Update(No::Registry& registry, float deltaTime) {
	No::Vector3 respawnPoint = No::Vector3::ZERO;
	for (auto e : registry.View<PlayerComponent>()) {
		respawnPoint = registry.GetComponent<PlayerComponent>(e)->respawnPoint;
	}


	for (auto e : registry.View<SavePointComponent>()) {
		auto* savePoint = registry.GetComponent<SavePointComponent>(e);
		if (savePoint->rotateTimer <= 0.0f) {
			savePoint->rotateTimer = 0.0f;
			registry.GetComponent<No::AnimatorComponent>(e)->animationSpeedMagnification = 1.0f;

			if (respawnPoint == registry.GetComponent<No::TransformComponent>(e)->GetWorldPosition(registry)) {
				registry.GetComponent<No::MaterialComponent>(e)->color = No::Color::GREEN * savePoint->colorMagnification;
			} else {
				registry.GetComponent<No::MaterialComponent>(e)->color = No::Color::WHITE * savePoint->colorMagnification;

			}
			continue;
		}
		savePoint->rotateTimer -= deltaTime;

	}
}
