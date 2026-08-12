#include "PlayerVerticalVelocitySystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"

void PlayerVerticalVelocitySystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<PlayerComponent, No::VelocityComponent, No::GroundStateComponent,
		No::ParticleEmitterSphereComponent, PlayerMoveTransientComponent>();

	for (auto entity : view) {
		auto* playerVariables = registry.GetComponent<PlayerComponent>(entity);
		auto* velocity = registry.GetComponent<No::VelocityComponent>(entity);
		auto* groundState = registry.GetComponent<No::GroundStateComponent>(entity);
		auto* particleEmitter = registry.GetComponent<No::ParticleEmitterSphereComponent>(entity);
		auto* transientState = registry.GetComponent<PlayerMoveTransientComponent>(entity);

		const bool isGrounded = groundState->isGrounded;
		const bool justJumped = transientState->justJumped;

		if (transientState->isAirDashing) {
			velocity->linear.y = 0.f;
			particleEmitter->active = false;
		} else if (isGrounded && !justJumped) {
			velocity->linear.y = transientState->slopeY;
		} else {
			playerVariables->yVelocity += playerVariables->gravity * deltaTime;
			velocity->linear.y = playerVariables->yVelocity;
			particleEmitter->active = false;
		}

		// ここでこのフレームの最終的なPlayerStateを決定する
		if (transientState->isAirDashing) {
			playerVariables->state = PlayerState::kAirDash;
		} else if (!isGrounded) {
			if (playerVariables->yVelocity > 0.f) {
				playerVariables->state =  PlayerState::kJump;
			} else {
				playerVariables->state = PlayerState::kFall;
			}
		} else {
			const bool isMoving = (velocity->linear.x != 0.f || velocity->linear.z != 0.f);
			playerVariables->state = isMoving ? PlayerState::kWalk : PlayerState::kWait;
		}

		playerVariables->groundNormal = No::Vector3::ZERO;
	}
}