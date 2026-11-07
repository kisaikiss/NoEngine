#include "PlayerHorizontalMoveSystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "PlayerStateMachine.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"
#include "application/ClockworksDisease/Component/Game/GoalDirectionComponent.h"
#include "../../Component/Camera/CameraIntroComponent.h"
#include "../../Component/Dialogue/DialogueComponent.h"
#include "../../Component/Player/PlayerDeathComponent.h"

namespace {

No::Quaternion GetActiveCameraRotation(No::Registry& registry) {
	No::Quaternion cameraRotate{};
	auto cameraView = registry.View<FollowCameraComponent>();
	for (auto entity : cameraView) {
		cameraRotate = registry.GetComponent<No::TransformComponent>(entity)->rotation;
	}
	return cameraRotate;
}

void FacePlayerTowardsMoveDirection(No::TransformComponent* transform, const No::Vector3& worldDir, float deltaTime) {
	const No::Vector3 lookDir = { worldDir.x, 0.f, worldDir.z };
	if (lookDir.Length() <= 1e-6f) {
		return;
	}
	No::Quaternion newRotation;
	newRotation.LookRotation(lookDir, No::Vector3::UP);
	constexpr float kSlerpScale = 20.f;
	transform->rotation = transform->rotation.Slerp(transform->rotation, newRotation, deltaTime * kSlerpScale);
}

// 水平方向の単位ベクトルを、地面の法線が作る平面上に投影する
No::Vector3 ProjectOnGroundPlane(const No::Vector3& horizontalDir, const No::Vector3& groundNormal) {
	No::Vector3 projected = horizontalDir - groundNormal * horizontalDir.Dot(groundNormal);
	const float len = projected.Length();
	if (len > 1e-6f) {
		return projected * (1.f / len);
	}
	return horizontalDir;
}

} // namespace

void PlayerHorizontalMoveSystem::Update(No::Registry& registry, float deltaTime) {
	const No::Quaternion cameraRotate = GetActiveCameraRotation(registry);

	auto view = registry.View<PlayerComponent, PlayerInputComponent, PlayerActionComponent,
		No::TransformComponent, No::VelocityComponent,
		No::GroundStateComponent, PlayerMoveTransientComponent>();

	bool isSkip = false;
	for (auto e : registry.View<No::TransformComponent, No::CameraComponent, FollowCameraComponent>()) {
		if (registry.Has<CameraLockTag>(e)) isSkip = true; // 演出中は操作できない
	}

	for (auto entity : view) {
		if (isSkip) continue;
		// 死亡後はリスポーン地点へ移動するまで操作を止める。
		// PlayerDeathSystemがkBlackOutへ進めた時点で位置が更新済みなので、暗転中から操作可能。
		auto* deathSequence = registry.GetComponent<PlayerDeathSequenceComponent>(entity);
		if (deathSequence && deathSequence->phase != PlayerDeathSequenceComponent::Phase::kBlackOut &&
			deathSequence->phase != PlayerDeathSequenceComponent::Phase::kFadeIn) {
			continue;
		}

		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* playerVariables = registry.GetComponent<PlayerComponent>(entity);
		auto* playerInput = registry.GetComponent<PlayerInputComponent>(entity);
		auto* playerAction = registry.GetComponent<PlayerActionComponent>(entity);

		// 水平移動の可否は現在Stateに委譲する。
		if (!PlayerStates::Get(playerVariables->state).CanMoveHorizontally()) continue;

		auto* velocity = registry.GetComponent<No::VelocityComponent>(entity);
		auto* groundState = registry.GetComponent<No::GroundStateComponent>(entity);
		auto* particleEmitterSphere = registry.GetComponent<No::ParticleEmitterSphereComponent>(entity);
		auto* particleEmitter = registry.GetComponent<No::ParticleEmitterComponent>(entity);
		auto* transientState = registry.GetComponent<PlayerMoveTransientComponent>(entity);
		auto* animator = registry.GetComponent<No::AnimatorComponent>(entity);

		if (registry.Has<GoalDirectionLockTag>(entity) || registry.Has<DialogueLockTag>(entity)) {
			velocity->linear = No::Vector3::ZERO;
			transientState->slopeY = 0.f;
			if (particleEmitterSphere) particleEmitterSphere->active = false;
			if (particleEmitter) particleEmitter->active = false;
			continue;
		}

		// 垂直速度は PlayerVerticalVelocitySystem が管理する。空中移動の慣性を
		// 保持するため、ここでは現在の水平速度だけを退避しておく。
		const No::Vector3 currentHorizontalVelocity = { velocity->linear.x, 0.f, velocity->linear.z };
		velocity->linear = No::Vector3::ZERO;
		No::Vector3 inputDir = No::Vector3::ZERO;
		inputDir = playerInput->moveAxis;

		// 倒した方向によらず同じ長さで判定し、キーボードの斜め入力は1に収める。
		const float inputForce = std::clamp(inputDir.Length(), 0.0f, 1.0f);

		const bool hasInput = (inputDir.x != 0.f || inputDir.z != 0.f);
		const bool isAirDashing = playerAction->activeAction == PlayerActionType::kAirDash;
		const bool isSpinAttacking = playerAction->activeAction == PlayerActionType::kSpinAttack;

		if (!hasInput && !isAirDashing) {
			if (particleEmitterSphere)
				particleEmitterSphere->active = false;
			if (particleEmitter)
				particleEmitter->active = false;
		} else if (particleEmitterSphere)
			particleEmitterSphere->active = true;
		if (hasInput || isAirDashing) {
			if (particleEmitter)
				particleEmitter->active = true;
		}

		const No::Vector3& groundNormal = playerVariables->groundNormal;

		No::Vector3 worldDir = No::Vector3::ZERO;
		if (hasInput) {
			worldDir = cameraRotate.RotateVector(inputDir);
			worldDir.y = 0.f;
		}

		const float len = worldDir.Length();
		No::Vector3 horizontalDir = (len > 1e-6f) ? worldDir * (1.f / len) : No::Vector3::ZERO;

		// 接地中（かつ空中ダッシュ中でない）なら、水平方向を地面平面へ投影して
		// 実際の斜面に沿った方向ベクトルにする
		No::Vector3 finalDir = horizontalDir;
		const bool followsGround = groundState->isGrounded && !transientState->justJumped && !isAirDashing;
		if (followsGround && horizontalDir.Length() > 1e-6f) {
			finalDir = ProjectOnGroundPlane(horizontalDir, groundNormal);
		}

		const bool isRunning = inputForce > playerVariables->dashStartInput;
		if (animator && !isSpinAttacking) {
			const bool isWalking = playerVariables->state == PlayerState::kWalk && !isRunning;
			animator->animationSpeedMagnification = isWalking ? 0.8f : 1.2f;
		}
		const float playerMoveSpeed = isRunning ? playerVariables->moveSpeed : playerVariables->walkSpeed;
		const float speed = isAirDashing ? playerVariables->airDashSpeed : playerMoveSpeed;
		// 加減速は水平成分だけで行い、上下成分は確定した水平速度から求める。
		const No::Vector3 targetVelocity{ finalDir.x * speed, 0.f, finalDir.z * speed };
		No::Vector3 finalVelocity = targetVelocity;

		if (!isAirDashing) {
			// 空中では入力を目標速度として扱い、加速量を制限して現在の速度から
			// 徐々に近づける。接地中は accelerationTime で最高速までの時間を調整する。
			// 入力を離した場合も同じく徐々に減速する。
			const No::Vector3 velocityDelta = targetVelocity - currentHorizontalVelocity;
			const float deltaLength = velocityDelta.Length();
			const float maxSpeedChange = groundState->isGrounded
				? (playerVariables->accelerationTime > 0.f
					? speed * deltaTime / playerVariables->accelerationTime
					: deltaLength)
				: playerVariables->airAcceleration * deltaTime;
			if (deltaLength > maxSpeedChange && deltaLength > 1e-6f) {
				finalVelocity = currentHorizontalVelocity + velocityDelta * (maxSpeedChange / deltaLength);
			}
		}

		velocity->linear.x = finalVelocity.x;
		velocity->linear.z = finalVelocity.z;

		// 加速中・減速中・方向転換中も、実際の移動速度を地面の接平面に合わせる。
		transientState->slopeY = followsGround && groundNormal.y > 1e-6f
			? -(finalVelocity.x * groundNormal.x + finalVelocity.z * groundNormal.z) / groundNormal.y
			: 0.f;

		// 回転攻撃中の見た目の回転はHurricaneKickに任せる。
		if (!isSpinAttacking) FacePlayerTowardsMoveDirection(transform, finalVelocity, deltaTime);
	}
}

REGISTER_SYSTEM(::PlayerHorizontalMoveSystem, "PlayerHorizontalMoveSystem", "Gameplay")
