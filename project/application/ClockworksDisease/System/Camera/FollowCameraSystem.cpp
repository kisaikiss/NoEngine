#include "FollowCameraSystem.h"
#include "../../Component/Player/PlayerComponent.h"
#include "../../Component/Camera/FollowCameraComponent.h"

void FollowCameraSystem::Update(No::Registry& registry, float deltaTime) {
	(void)deltaTime;
	No::Vector3 playerPos{};
	auto playerView = registry.View<No::TransformComponent, PlayerComponent>();
	for (auto e : playerView) {
		playerPos = registry.GetComponent<No::TransformComponent>(e)->GetWorldPosition(registry);
	}

	auto view = registry.View<No::TransformComponent, FollowCameraComponent>();

	for (auto e : view) {
		auto* transform = registry.GetComponent<No::TransformComponent>(e);
		auto* followCameraVariables = registry.GetComponent<FollowCameraComponent>(e);

		float hInput = No::GetInputAxisValue("CameraHorizontal");
		float vInput = No::GetInputAxisValue("CameraVertical");

		followCameraVariables->theta += hInput * deltaTime;
		followCameraVariables->phi += vInput * deltaTime;

		// 移動制限
		if (followCameraVariables->phi >= PI) {
			followCameraVariables->phi = PI - 0.001f;
		} else if (followCameraVariables->phi <= PI /10.f) {
			followCameraVariables->phi = PI / 10.f;
		}


		No::Vector3 nextPosition;


		nextPosition.x = playerPos.x + followCameraVariables->distance * std::sin(followCameraVariables->phi) * std::cos(followCameraVariables->theta);
		nextPosition.y = playerPos.y + followCameraVariables->distance * std::cos(followCameraVariables->phi);
		nextPosition.z = playerPos.z + followCameraVariables->distance * std::sin(followCameraVariables->phi) * std::sin(followCameraVariables->theta);

		transform->translate = No::Lerp(transform->translate, nextPosition, 0.1f);

		transform->rotation.LookRotation(playerPos - transform->GetWorldPosition(registry), No::Vector3::UP);
		
		
	}



}
