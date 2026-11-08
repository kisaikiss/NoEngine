#include "PlayerActionSystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerMoveTags.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"
#include "application/ClockworksDisease/Component/Game/GoalDirectionComponent.h"
#include "application/ClockworksDisease/Component/Dialogue/DialogueComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerDeathComponent.h"
#include "../../Component/Camera/CameraIntroComponent.h"

void PlayerActionSystem::Update(No::Registry& registry, float deltaTime) {
	if (deltaTime <= 0.0f) return;
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
		auto* attackSensor = registry.GetComponent<No::CapsuleSensorComponent>(entity);
		if (!attackSensor) attackSensor = registry.AddComponent<No::CapsuleSensorComponent>(entity);
		attackSensor->enabled = false;
		attackSensor->radius = player->spinAttackRadius;

		const auto* death = registry.GetComponent<PlayerDeathSequenceComponent>(entity);
		const bool deathLocked = death
			&& death->phase != PlayerDeathSequenceComponent::Phase::kBlackOut
			&& death->phase != PlayerDeathSequenceComponent::Phase::kFadeIn;
		const bool actionLocked = cameraLocked || deathLocked
			|| registry.Has<GoalDirectionLockTag>(entity)
			|| registry.Has<DialogueLockTag>(entity);
		auto* animator = registry.GetComponent<No::AnimatorComponent>(entity);
		if (action->activeAction == PlayerActionType::kSpinAttack) {
			if (!actionLocked && animator && animator->IsPlaying()) {
				attackSensor->enabled = true;
				action->elapsedTime += deltaTime;
				continue;
			}
			if (animator) animator->animationSpeedMagnification = action->animationSpeedBeforeSpinAttack;
			action->activeAction = PlayerActionType::kNone;
			action->elapsedTime = 0.0f;
		}
		if (actionLocked) continue;
		const auto* move = registry.GetComponent<PlayerMoveTransientComponent>(entity);
		if (move && (move->crouching || move->backflipPhase != PlayerBackflipPhase::kNone)) continue;

		// 押した瞬間だけ開始し、連打で再生途中の攻撃を巻き戻さない。
		if (input->attackTriggered && animator) {
			const float previousSpeed = animator->animationSpeedMagnification;
			const float attackSpeed = player->spinAttackAnimationSpeed;
			// 0や不正な値で再生が止まり、攻撃が終了しなくなるのを防ぐ。
			animator->animationSpeedMagnification = std::isfinite(attackSpeed) && attackSpeed > 0.0f ? attackSpeed : 1.0f;
			if (animator->Play("HurricaneKick", false)) {
				action->animationSpeedBeforeSpinAttack = previousSpeed;
				attackSensor->enabled = true;
				action->activeAction = PlayerActionType::kSpinAttack;
				action->elapsedTime = 0.0f;
				continue;
			}
			animator->animationSpeedMagnification = previousSpeed;
		}

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
