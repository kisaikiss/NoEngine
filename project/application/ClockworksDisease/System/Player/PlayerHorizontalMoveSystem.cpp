#include "PlayerHorizontalMoveSystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"

namespace {

No::Quaternion GetActiveCameraRotation(No::Registry& registry) {
	No::Quaternion cameraRotate{};
	auto cameraView = registry.View<FollowCameraComponent>();
	for (auto entity : cameraView) {
		cameraRotate = registry.GetComponent<No::TransformComponent>(entity)->rotation;
	}
	return cameraRotate;
}

// 現在の向きの正面ベクトル（水平成分のみ）。
No::Vector3 GetFacingDirection(const No::TransformComponent* transform) {
	No::Vector3 forward = transform->rotation.RotateVector(No::Vector3::FORWARD);
	forward.y = 0.f;
	return forward;
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
        No::GroundStateComponent, No::ParticleEmitterSphereComponent, PlayerMoveTransientComponent>();

    for (auto entity : view) {
        auto* transform = registry.GetComponent<No::TransformComponent>(entity);
        auto* playerVariables = registry.GetComponent<PlayerComponent>(entity);
        auto* velocity = registry.GetComponent<No::VelocityComponent>(entity);
        auto* groundState = registry.GetComponent<No::GroundStateComponent>(entity);  
        auto* particleEmitter = registry.GetComponent<No::ParticleEmitterSphereComponent>(entity);
        auto* transientState = registry.GetComponent<PlayerMoveTransientComponent>(entity);

        velocity->linear = No::Vector3::ZERO;

        No::Vector3 inputDir = No::Vector3::ZERO;
        inputDir.x = No::GetInputAxisValue("Lateral");
        inputDir.z = No::GetInputAxisValue("Forward");

        const bool hasInput = (inputDir.x != 0.f || inputDir.z != 0.f);
        const bool isAirDashing = transientState->isAirDashing;

        if (!hasInput && !isAirDashing) {
            particleEmitter->active = false;
            transientState->slopeY = 0.f;
            continue;
        }

        particleEmitter->active = true;

        const No::Vector3& groundNormal = playerVariables->groundNormal;

        No::Vector3 worldDir;
        if (hasInput) {
            worldDir = cameraRotate.RotateVector(inputDir);
            worldDir.y = 0.f;
        } else {
            worldDir = GetFacingDirection(transform);
        }

        const float len = worldDir.Length();
        No::Vector3 horizontalDir = (len > 1e-6f) ? worldDir * (1.f / len) : No::Vector3::ZERO;

        // 接地中（かつ空中ダッシュ中でない）なら、水平方向を地面平面へ投影して
        // 実際の斜面に沿った方向ベクトルにする
        No::Vector3 finalDir = horizontalDir;
        if (groundState->isGrounded && !isAirDashing && horizontalDir.Length() > 1e-6f) {
            finalDir = ProjectOnGroundPlane(horizontalDir, groundNormal);
        }

        const float speed = isAirDashing ? playerVariables->airDashSpeed : playerVariables->moveSpeed;
        No::Vector3 finalVelocity = finalDir * speed;

        velocity->linear.x = finalVelocity.x;
        velocity->linear.z = finalVelocity.z;

        // 斜面追従によるy成分を記録し、PlayerVerticalVelocitySystemへ渡す
        transientState->slopeY = finalVelocity.y;

        FacePlayerTowardsMoveDirection(transform, finalVelocity, deltaTime);
    }
}