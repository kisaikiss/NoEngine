#include "PlayerHorizontalMoveSystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"

namespace {

No::Quaternion GetActiveCameraRotation(No::Registry& registry) {
	No::Quaternion cameraRotate{};
	auto cameraView = registry.View<No::ActiveCameraTag, No::TransformComponent, FollowCameraComponent>();
	for (auto entity : cameraView) {
		cameraRotate = registry.GetComponent<No::TransformComponent>(entity)->rotation;
	}
	return cameraRotate;
}

void FacePlayerTowardsMoveDirection(No::TransformComponent* transform, const No::Vector3& worldDir,
	const No::Vector3& groundNormal, float deltaTime) {
	const No::Vector3 lookDir = { worldDir.x, 0.f, worldDir.z };
	if (lookDir.Length() <= 1e-6f) {
		return;
	}
	No::Quaternion newRotation;
	newRotation.LookRotation(lookDir, groundNormal);
	constexpr float kSlerpScale = 20.f;
	transform->rotation = transform->rotation.Slerp(transform->rotation, newRotation, deltaTime * kSlerpScale);
}

} // namespace

void PlayerHorizontalMoveSystem::Update(No::Registry& registry, float deltaTime) {
	No::Quaternion cameraRotate = GetActiveCameraRotation(registry);

	auto view = registry.View<PlayerComponent, No::TransformComponent, No::VelocityComponent,
		No::GroundStateComponent, No::ParticleEmitterComponent, PlayerMoveTransientComponent>();

	for (auto entity : view) {
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* playerVariables = registry.GetComponent<PlayerComponent>(entity);
		auto* velocity = registry.GetComponent<No::VelocityComponent>(entity);
		auto* groundState = registry.GetComponent<No::GroundStateComponent>(entity);
		auto* particleEmitter = registry.GetComponent<No::ParticleEmitterComponent>(entity);
		auto* transientState = registry.GetComponent<PlayerMoveTransientComponent>(entity);

		// このフレームの水平/垂直velocityをまとめてリセット
		// （yはこの後 PlayerVerticalVelocitySystem が上書きするが、初期化はここで行う）
		velocity->linear = No::Vector3::ZERO;

		No::Vector3 inputDir = No::Vector3::ZERO;
		inputDir.x = No::GetInputAxisValue("Lateral");
		inputDir.z = No::GetInputAxisValue("Forward");

		if (inputDir.x == 0.f && inputDir.z == 0.f) {
			particleEmitter->active = false;
			transientState->slopeY = 0.f;
			continue;
		}

		particleEmitter->active = true;

		const No::Vector3& groundNormal = playerVariables->groundNormal;
		const bool isGrounded = groundState->isGrounded;
		const bool justJumped = transientState->justJumped;

		// カメラ基準の移動ベクトル（水平成分のみ）
		No::Vector3 worldDir = cameraRotate.RotateVector(inputDir);
		worldDir.y = 0.f;

		if (isGrounded && !justJumped) {
			// 斜面に沿わせるため、法線方向の成分を除去して接平面に投影
			const float dn = worldDir.Dot(groundNormal);
			worldDir = worldDir - groundNormal * dn;
		}

		// 正規化してスピードを乗算
		const float len = worldDir.Length();
		if (len > 1e-6f) {
			worldDir = worldDir * (playerVariables->moveSpeed / len);
		}

		velocity->linear.x = worldDir.x;
		velocity->linear.z = worldDir.z;

		// 斜面寄与のy成分を記録し、後段の PlayerVerticalVelocitySystem に渡す
		transientState->slopeY = worldDir.y;

		FacePlayerTowardsMoveDirection(transform, worldDir, groundNormal, deltaTime);
	}
}
