#include "PlayerVerticalVelocitySystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "application/ClockworksDisease/Component/Game/GoalDirectionComponent.h"
#include "PlayerStateMachine.h"

void PlayerVerticalVelocitySystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<PlayerComponent, No::VelocityComponent, No::GroundStateComponent,
		No::ParticleEmitterSphereComponent, PlayerMoveTransientComponent, PlayerActionComponent>();

	for (auto entity : view) {
		auto* playerVariables = registry.GetComponent<PlayerComponent>(entity);
		auto* velocity = registry.GetComponent<No::VelocityComponent>(entity);
		auto* groundState = registry.GetComponent<No::GroundStateComponent>(entity);
		auto* particleEmitter = registry.GetComponent<No::ParticleEmitterSphereComponent>(entity);
		auto* transientState = registry.GetComponent<PlayerMoveTransientComponent>(entity);
		auto* playerAction = registry.GetComponent<PlayerActionComponent>(entity);

		if (registry.Has<GoalDirectionLockTag>(entity)) {
			particleEmitter->active = false;
		}

		const bool isGrounded = groundState->isGrounded;
		const bool justJumped = transientState->justJumped;

		const bool isAirDashing = playerAction->activeAction == PlayerActionType::kAirDash;
		// 接地した歩行だけ追従を予約する。段差や崖への吸着は短い距離に制限する。
		groundState->snapDistance = 0.f;
		groundState->snapMinNormalY = playerVariables->minGroundNormalY;
		if (isGrounded && !justJumped && !isAirDashing) {
			const No::Vector3 horizontalVelocity{ velocity->linear.x, 0.f, velocity->linear.z };
			groundState->snapDistance = std::min(0.25f, horizontalVelocity.Length() * deltaTime + 0.002f);
		}
		if (isAirDashing) {
			velocity->linear.y = 0.f;
			particleEmitter->active = false;
		} else if (isGrounded && !justJumped) {
			velocity->linear.y = transientState->slopeY;
		} else {
			playerVariables->yVelocity += playerVariables->gravity * deltaTime;
			if (playerVariables->yVelocity < playerVariables->maxFallSpeed) {
				playerVariables->yVelocity = playerVariables->maxFallSpeed;
			}
			velocity->linear.y = playerVariables->yVelocity;
			particleEmitter->active = false;
		}

		const bool isMoving = (velocity->linear.x != 0.f || velocity->linear.z != 0.f);
		PlayerStateContext stateContext{ *playerVariables, isGrounded, isMoving, isAirDashing };
		PlayerStates::Update(stateContext);

		playerVariables->groundNormal = No::Vector3::ZERO;
	}
}

REGISTER_SYSTEM(::PlayerVerticalVelocitySystem, "PlayerVerticalVelocitySystem", "Gameplay")
