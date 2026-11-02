#include "PlayerActionSystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerMoveTags.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"
#include "application/ClockworksDisease/Component/Game/GoalDirectionComponent.h"
#include "application/ClockworksDisease/Component/Dialogue/DialogueComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerDeathComponent.h"
#include "../../Component/Camera/CameraIntroComponent.h"

void PlayerActionSystem::Update(No::Registry& registry, float deltaTime) {
	bool cameraLocked = false;
	for (auto entity : registry.View<No::TransformComponent, No::CameraComponent, FollowCameraComponent>()) {
		if (registry.Has<CameraLockTag>(entity)) cameraLocked = true;
	}

	auto view = registry.View<PlayerComponent, PlayerInputComponent, PlayerActionComponent,
		No::GroundStateComponent>();
	for (auto entity : view) {
		auto* player = registry.GetComponent<PlayerComponent>(entity);
		auto* input = registry.GetComponent<PlayerInputComponent>(entity);
		auto* action = registry.GetComponent<PlayerActionComponent>(entity);
		auto* ground = registry.GetComponent<No::GroundStateComponent>(entity);

		const auto* death = registry.GetComponent<PlayerDeathSequenceComponent>(entity);
		const bool deathLocked = death
			&& death->phase != PlayerDeathSequenceComponent::Phase::kBlackOut
			&& death->phase != PlayerDeathSequenceComponent::Phase::kFadeIn;
		const bool actionLocked = cameraLocked || deathLocked
			|| registry.Has<GoalDirectionLockTag>(entity)
			|| registry.Has<DialogueLockTag>(entity);
		if (actionLocked) continue;

		if (action->activeAction == PlayerActionType::kAirDash) {
			if (input->airDashReleased) player->yVelocity = 0.0f;
			if (ground->isGrounded || !input->airDashPressed
				|| !registry.Has<AirDashTag>(entity)
				|| player->stamina <= player->airDashStaminaCostRate * deltaTime) {
				action->activeAction = PlayerActionType::kNone;
				action->elapsedTime = 0.0f;
			} else {
				action->elapsedTime += deltaTime;
				player->stamina -= player->airDashStaminaCostRate * deltaTime;
			}
			continue;
		}

		if (ground->isGrounded || !input->airDashPressed
			|| !registry.Has<AirDashTag>(entity)
			|| player->stamina <= player->airDashStaminaCostRate * deltaTime) {
			continue;
		}

		action->activeAction = PlayerActionType::kAirDash;
		action->elapsedTime = 0.0f;
		player->stamina -= player->airDashStaminaCostRate * deltaTime;
	}
}

REGISTER_SYSTEM(::PlayerActionSystem, "PlayerActionSystem", "Gameplay")
