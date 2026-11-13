#include "PlayerActionSystem.h"
#include "PlayerStateMachine.h"
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
		auto* move = registry.GetComponent<PlayerMoveTransientComponent>(entity);
		const auto backflipPhase = move ? move->backflipPhase : PlayerBackflipPhase::kNone;
		const bool dashMovementLocked = (move && move->crouching)
			|| (backflipPhase != PlayerBackflipPhase::kNone && backflipPhase != PlayerBackflipPhase::kAirborne);

		if (action->activeAction == PlayerActionType::kAirDash) {
			if (!actionLocked && !dashMovementLocked && !ground->isGrounded
				&& action->elapsedTime < player->airDashDuration
				&& action->airDashDirection.LengthSquared() > 0.f
				&& registry.Has<AirDashTag>(entity)) {
				continue;
			}
			action->activeAction = PlayerActionType::kNone;
			action->elapsedTime = 0.f;
			action->airDashDirection = No::Vector3::ZERO;
			if (!move || !move->justJumped) player->yVelocity = 0.f;
			// ダッシュの速度を通常の空中移動に持ち越さない。
			if (auto* velocity = registry.GetComponent<No::VelocityComponent>(entity)) {
				velocity->linear.x = 0.f;
				velocity->linear.z = 0.f;
			}
		}
		if (actionLocked || dashMovementLocked) continue;

		// 押した瞬間だけ開始し、連打で再生途中の攻撃を巻き戻さない。
		if (backflipPhase == PlayerBackflipPhase::kNone && input->attackTriggered && animator) {
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

		if (!input->airDashTriggered || ground->isGrounded
			|| !registry.Has<AirDashTag>(entity)
			|| !PlayerStates::Get(player->state).CanMoveHorizontally()
			|| !std::isfinite(player->airDashDuration) || player->airDashDuration <= 0.f
			|| !std::isfinite(player->airDashSpeed) || player->airDashSpeed <= 0.f) {
			continue;
		}
		const float staminaCost = player->airDashStaminaCost;
		if (!std::isfinite(staminaCost) || staminaCost < 0.f || player->stamina < staminaCost) continue;

		const auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		if (!transform) continue;
		No::Vector3 direction = GetPlayerMovementCameraRotation(registry, input->moveAxis).RotateVector(input->moveAxis);
		direction.y = 0.f;
		float length = direction.Length();
		if (length <= 1e-6f) {
			direction = transform->GetWorldRotation(registry).RotateVector(No::Vector3::FORWARD);
			direction.y = 0.f;
			length = direction.Length();
		}
		if (!std::isfinite(length) || length <= 1e-6f) continue;

		// 発動できたときだけ宙返りを打ち切り、ダッシュの移動とアニメーションに切り替える。
		if (backflipPhase == PlayerBackflipPhase::kAirborne) {
			move->backflipPhase = PlayerBackflipPhase::kNone;
		}
		action->activeAction = PlayerActionType::kAirDash;
		action->elapsedTime = 0.f;
		action->airDashDirection = direction * (1.f / length);
		player->stamina -= staminaCost;
	}
}

REGISTER_SYSTEM(::PlayerActionSystem, "PlayerActionSystem", "Gameplay")
