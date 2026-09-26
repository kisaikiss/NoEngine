#include "stdafx.h"
#include "PlayerDeathSystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"

void PlayerDeathSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	bool isDeath = false;
	No::Vector3 playerPos = No::Vector3::ZERO;
	for (auto e : registry.View<PlayerComponent>()) {
		auto* player = registry.GetComponent<PlayerComponent>(e);
		auto* transform = registry.GetComponent<No::TransformComponent>(e);
		if (transform->GetWorldPosition(registry).y < player->deathHeight) {
			transform->translate = player->respawnPoint;
			playerPos = transform->GetWorldPosition(registry);
			player->yVelocity = 0.0f;
			registry.GetComponent<No::VelocityComponent>(e)->linear = No::Vector3::ZERO;
			isDeath = true;
		}
	}

	if (isDeath) {
		for (auto e : registry.View<FollowCameraComponent>()) {
			auto* camera = registry.GetComponent<FollowCameraComponent>(e);
			auto* transform = registry.GetComponent<No::TransformComponent>(e);
			camera->phi = 1.0f;

			transform->translate.y = playerPos.y + camera->distance * std::cos(camera->phi);
		}
	}
}
