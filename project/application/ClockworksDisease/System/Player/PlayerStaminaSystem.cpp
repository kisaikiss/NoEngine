#include "stdafx.h"
#include "PlayerStaminaSystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"

void PlayerStaminaSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<PlayerComponent, No::GroundStateComponent>();
	for (auto entity : view) {
		auto* playerVariables = registry.GetComponent<PlayerComponent>(entity);
		auto* groundState = registry.GetComponent<No::GroundStateComponent>(entity);

		float& stamina = playerVariables->stamina;
		const float maxStamina = playerVariables->maxStamina;

		if (groundState->isGrounded && stamina < maxStamina) {
			stamina += deltaTime * playerVariables->staminaRecoveryRate;
		}
		if (stamina > maxStamina) {
			stamina = maxStamina;
		}

#ifdef USE_IMGUI
		if (playerVariables->infinityStamina) {
			stamina = FLT_MAX;
		}
#endif // USE_IMGUI

	
	}
}
