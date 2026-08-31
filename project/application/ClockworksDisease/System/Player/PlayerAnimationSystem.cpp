#include "stdafx.h"
#include "PlayerAnimationSystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"

void PlayerAnimationSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto view = registry.View<PlayerComponent, No::AnimatorComponent>();

	for (auto entity : view) {
		auto* playerVariables = registry.GetComponent<PlayerComponent>(entity);
		auto* animator = registry.GetComponent<No::AnimatorComponent>(entity);

		switch (playerVariables->state) {
			case PlayerState::kWait:
				animator->currentAnimation = 1;
				break;
			case PlayerState::kFall:
			case PlayerState::kJump:
			case PlayerState::kHighJump:
				animator->currentAnimation = 0;
				break;
			case PlayerState::kWalk:
				animator->currentAnimation = 2;
				break;
		}

	}

}
