#include "stdafx.h"
#include "RabbitdokuPushBackSystem.h"
#include "../../Component/RabbitdokuComponent.h"
#include "../../Game/RabbitdokuCollisionLayer.h"

void RabbitdokuPushBackSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto view = registry.PollAllEvents<RabbitdokuPushBackEvent>();
	for (auto event : view) {
		if (event.position == No::ContactPosition::UP) {
			auto* player = registry.GetComponent<Rabbitdoku>(event.player);
			auto* groundState = registry.GetComponent<No::GroundStateComponent>(event.player);
			if (player->yVelocity > 0.f) {
				player->yVelocity = 0.f;
			}
			player->canDoubleJump = true;
			groundState->isGrounded = true;
		}
		if (event.position == No::ContactPosition::DOWN) {
			auto* player = registry.GetComponent<Rabbitdoku>(event.player);
			if (player->yVelocity < 0.f) {
				player->yVelocity = 0.f;
			}
		}
	}
}
