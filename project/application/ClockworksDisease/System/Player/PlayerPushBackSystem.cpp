#include "PlayerPushBackSystem.h"
#include "../Game/CollisionEvents.h"
#include "../../Component/Player/PlayerComponent.h"

void PlayerPushBackSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto view = registry.PollAllEvents<PlayerPushBackEvent>();
	for (const auto& event : view) {
		if (event.position == No::ContactPosition::UP) {
			auto* player = registry.GetComponent<PlayerComponent>(event.player);
			if (player->yVelocity < 0.f) {
				player->yVelocity = 0.f;
			}

			auto* ground = registry.GetComponent<No::GroundStateComponent>(event.player);
			ground->isGrounded = true;
			ground->groundHeight = registry.GetComponent<No::TransformComponent>(event.player)->GetWorldPosition().y;
		}
		if (event.position == No::ContactPosition::DOWN) {
			auto* player = registry.GetComponent<PlayerComponent>(event.player);
			if (player->yVelocity > 0.f) {
				player->yVelocity = 0.f;
			}

		}
	

	}
}
