#include "PlayerHorizontalMoveSystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"
#include "application/ClockworksDisease/Component/Game/GoalDirectionComponent.h"
#include "../../Component/Camera/CameraIntroComponent.h"

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

	auto view = registry.View<PlayerComponent, No::TransformComponent, No::VelocityComponent,
		No::GroundStateComponent, PlayerMoveTransientComponent>();

	bool isSkip = false;
	for (auto e : registry.View<No::TransformComponent, No::CameraComponent, FollowCameraComponent>()) {
		if (registry.Has<CameraIntroLockTag>(e)) isSkip = true; // 演出中は操作できない
	}

	for (auto entity : view) {
		if (isSkip) continue;

		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* playerVariables = registry.GetComponent<PlayerComponent>(entity);
		auto* velocity = registry.GetComponent<No::VelocityComponent>(entity);
		auto* groundState = registry.GetComponent<No::GroundStateComponent>(entity);
		auto* particleEmitterSphere = registry.GetComponent<No::ParticleEmitterSphereComponent>(entity);
		auto* particleEmitter = registry.GetComponent<No::ParticleEmitterComponent>(entity);
		auto* transientState = registry.GetComponent<PlayerMoveTransientComponent>(entity);
		auto* animator = registry.GetComponent<No::AnimatorComponent>(entity);

		if (registry.Has<GoalDirectionLockTag>(entity)) {
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
		inputDir.x = No::GetInputAxisValue("Lateral");
		inputDir.z = No::GetInputAxisValue("Forward");

		float inputForce = (std::fabsf(inputDir.x) + std::fabsf(inputDir.z)) / 2.0f;

		const bool hasInput = (inputDir.x != 0.f || inputDir.z != 0.f);
		const bool isAirDashing = transientState->isAirDashing;

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
		if (groundState->isGrounded && !isAirDashing && horizontalDir.Length() > 1e-6f) {
			finalDir = ProjectOnGroundPlane(horizontalDir, groundNormal);
		}

		if (playerVariables->state == PlayerState::kWalk && inputForce < playerVariables->dashStartInput) {
			animator->animationSpeedMagnification = 0.8f;
		} else {
			animator->animationSpeedMagnification = 1.2f;
		}
		const float playerMoveSpeed = inputForce > playerVariables->dashStartInput ? playerVariables->moveSpeed : playerVariables->walkSpeed;
		const float speed = isAirDashing ? playerVariables->airDashSpeed : playerMoveSpeed;
		const No::Vector3 targetVelocity = finalDir * speed;
		No::Vector3 finalVelocity = targetVelocity;

		if (!groundState->isGrounded && !isAirDashing) {
			// 空中では入力を目標速度として扱い、加速量を制限して現在の速度から
			// 徐々に近づける。入力を離した場合も同じく徐々に減速する。
			const No::Vector3 velocityDelta = targetVelocity - currentHorizontalVelocity;
			const float deltaLength = velocityDelta.Length();
			const float maxSpeedChange = playerVariables->airAcceleration * deltaTime;
			if (deltaLength > maxSpeedChange && deltaLength > 1e-6f) {
				finalVelocity = currentHorizontalVelocity + velocityDelta * (maxSpeedChange / deltaLength);
			}
		}

		velocity->linear.x = finalVelocity.x;
		velocity->linear.z = finalVelocity.z;

		// 斜面追従によるy成分を記録し、PlayerVerticalVelocitySystemへ渡す
		transientState->slopeY = finalVelocity.y;

		FacePlayerTowardsMoveDirection(transform, finalVelocity, deltaTime);
	}
}
