#include "FollowCameraSystem.h"
#include "../../Component/Player/PlayerComponent.h"
#include "../../Component/Camera/FollowCameraComponent.h"

namespace {
// 角度差を -PI 〜 PI にラップする(最短経路で回転させるため)
float WrapAngle(float angle) {
	while (angle > PI)  angle -= 2.f * PI;
	while (angle < -PI) angle += 2.f * PI;
	return angle;
}
}

void FollowCameraSystem::Update(No::Registry& registry, float deltaTime) {
	(void)deltaTime;
	No::Vector3 playerPos{};
	No::Vector3 playerVelocity{};
	auto playerView = registry.View<PlayerComponent>();
	for (auto e : playerView) {
		playerPos = registry.GetComponent<No::TransformComponent>(e)->GetWorldPosition(registry);
		playerVelocity = registry.GetComponent<No::VelocityComponent>(e)->linear;
	}

	auto view = registry.View<FollowCameraComponent>();

	for (auto e : view) {
		auto* transform = registry.GetComponent<No::TransformComponent>(e);
		auto* followCameraVariables = registry.GetComponent<FollowCameraComponent>(e);

		float hInput = No::GetInputAxisValue("CameraHorizontal");
		float vInput = No::GetInputAxisValue("CameraVertical");

		float pInput = No::GetInputAxisValue("Lateral") + No::GetInputAxisValue("Forward");

		followCameraVariables->theta += hInput * deltaTime;
		followCameraVariables->phi += vInput * deltaTime;

		if (hInput || vInput) {
			// カメラ操作中はタイマーリセット
			followCameraVariables->idleTimer = 0.0f;
		} else {
			if (pInput) {
				// カメラ操作がされておらず、プレイヤー操作があるときはタイマーを進める
				//followCameraVariables->idleTimer += deltaTime;

			}
		}

		// 移動制限
		if (followCameraVariables->phi >= PI) {
			followCameraVariables->phi = PI - 0.001f;
		} else if (followCameraVariables->phi <= PI /10.f) {
			followCameraVariables->phi = PI / 10.f;
		}

		if (followCameraVariables->hasPrevPlayerPos) {
			No::Vector3 delta = playerPos - followCameraVariables->prevPlayerPos;
			delta.y = 0.f; // 水平面のみ
			float speed = delta.Length() / std::max(deltaTime, 1e-5f);

			// 一定以上の速さで動いているときだけ移動方向を更新・平滑化
			const float moveSpeedThreshold = 0.1f;
			if (speed > moveSpeedThreshold) {
				No::Vector3 dir = delta.Normalize();
				// ローパスフィルタでなめらかに追従(急な切り返しを緩和)
				float smoothT = 1.f - std::exp(-4.0f * deltaTime);
				followCameraVariables->smoothedMoveDir = No::Lerp(followCameraVariables->smoothedMoveDir, dir, smoothT).Normalize();
			}
		}
		followCameraVariables->prevPlayerPos = playerPos;
		followCameraVariables->hasPrevPlayerPos = true;

		// --- 自動追従(移動方向の背後へ、最大角速度でクランプしつつ回り込む) ---
		if (followCameraVariables->idleTimer >= followCameraVariables->autoFollowDelay && followCameraVariables->smoothedMoveDir.LengthSquared() > 0.0001f) {
			float targetTheta = std::atan2(followCameraVariables->smoothedMoveDir.z, followCameraVariables->smoothedMoveDir.x) + PI;
			float diff = WrapAngle(targetTheta - followCameraVariables->theta);

			if (std::abs(diff) > followCameraVariables->autoFollowDeadzoneAngle) {
				float maxStep = followCameraVariables->maxAutoFollowAngularSpeed * deltaTime;
				float step = std::clamp(diff, -maxStep, maxStep); // 急回転を防ぐ
				followCameraVariables->theta += step;
			}
		}
		No::Vector3 nextPosition;


		nextPosition.x = playerPos.x + followCameraVariables->distance * std::sin(followCameraVariables->phi) * std::cos(followCameraVariables->theta);
		nextPosition.y = playerPos.y + followCameraVariables->distance * std::cos(followCameraVariables->phi);
		nextPosition.z = playerPos.z + followCameraVariables->distance * std::sin(followCameraVariables->phi) * std::sin(followCameraVariables->theta);

		transform->translate = No::Lerp(transform->translate, nextPosition, 0.1f);

		transform->rotation.LookRotation(playerPos - transform->GetWorldPosition(registry), No::Vector3::UP);
		
		
	}



}
