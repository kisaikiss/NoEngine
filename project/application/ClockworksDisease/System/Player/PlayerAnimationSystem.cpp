#include "stdafx.h"
#include "PlayerAnimationSystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "PlayerStateMachine.h"

void PlayerAnimationSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto view = registry.View<PlayerComponent, No::AnimatorComponent>();

	for (auto entity : view) {
		auto* playerVariables = registry.GetComponent<PlayerComponent>(entity);
		auto* animator = registry.GetComponent<No::AnimatorComponent>(entity);

		const bool moving = false;
		PlayerStateContext stateContext{ *playerVariables, false, moving, false };
		animator->currentAnimation = PlayerStates::Get(playerVariables->state).AnimationIndex(stateContext);

	}

}

REGISTER_SYSTEM(::PlayerAnimationSystem, "PlayerAnimationSystem", "Gameplay")
