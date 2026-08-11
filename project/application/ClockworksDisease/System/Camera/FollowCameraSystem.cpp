#include "FollowCameraSystem.h"
#include "../../Component/Player/PlayerComponent.h"
#include "../../Component/Camera/FollowCameraComponent.h"

void FollowCameraSystem::Update(No::Registry& registry, float deltaTime) {
	(void)deltaTime;
	No::Vector3 playerPos{};
	auto playerView = registry.View<No::TransformComponent, No::VelocityComponent, PlayerComponent>();
	for (auto e : playerView) {
		playerPos = registry.GetComponent<No::TransformComponent>(e)->GetWorldPosition(registry);
	}

	auto view = registry.View<No::TransformComponent, No::CameraComponent, FollowCameraComponent>();

	for (auto e : view) {
		auto* transform = registry.GetComponent<No::TransformComponent>(e);
		auto* followCameraVariables = registry.GetComponent<FollowCameraComponent>(e);
		auto* cameraComponent = registry.GetComponent<No::CameraComponent>(e);

		float hInput = No::GetInputAxisValue("CameraHorizontal");
		float vInput = No::GetInputAxisValue("CameraVertical");

		followCameraVariables->phi += vInput * deltaTime * followCameraVariables->moveSpeed;
		followCameraVariables->theta += hInput * deltaTime * followCameraVariables->moveSpeed;


		const float& minPhi = followCameraVariables->minPhi;
		const float& maxPhi = followCameraVariables->maxPhi;

		// 移動制限
		if (followCameraVariables->phi >= maxPhi) {
			followCameraVariables->phi = maxPhi;
		} else if (followCameraVariables->phi <= minPhi) {
			followCameraVariables->phi = minPhi;
		}

		// Phi(縦方向の移動)によって「プレイヤーとカメラの距離」と「視野角」を調整する
		float normalizedPhi = (followCameraVariables->phi - minPhi) / (maxPhi - minPhi);
		followCameraVariables->distance = No::Lerp(followCameraVariables->maxDistance, 2.0f, normalizedPhi);

		cameraComponent->fov = No::Lerp(followCameraVariables->minFov, followCameraVariables->maxFov, normalizedPhi);


		// 次行くべきカメラ座標の計算
		No::Vector3 nextPosition;

		nextPosition.x = playerPos.x + followCameraVariables->distance * std::sin(followCameraVariables->phi) * std::cos(followCameraVariables->theta);
		nextPosition.y = playerPos.y + followCameraVariables->distance * std::cos(followCameraVariables->phi);
		nextPosition.z = playerPos.z + followCameraVariables->distance * std::sin(followCameraVariables->phi) * std::sin(followCameraVariables->theta);


		// 実際の移動
		constexpr float moveTime = 0.1f;
		No::Vector3 nowTranslate = transform->translate;
		transform->translate = No::Lerp(nowTranslate, nextPosition, moveTime);
		
		// プレイヤー + Offset座標を見る
		playerPos.y += followCameraVariables->playerPosOffset;
		transform->rotation.LookRotation(playerPos - transform->GetWorldPosition(registry), No::Vector3::UP);
	
	}



}
