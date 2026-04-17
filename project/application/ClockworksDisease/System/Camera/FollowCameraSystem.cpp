#include "FollowCameraSystem.h"
#include "../../Component/Player/PlayerComponent.h"
#include "../../Component/Camera/FollowCameraComponent.h"

void FollowCameraSystem::Update(No::Registry& registry, float deltaTime) {
	(void)deltaTime;
	No::Vector3 playerPos{};

	auto playerView = registry.View<PlayerComponent>();
	for (auto e : playerView) {
		playerPos = registry.GetComponent<No::TransformComponent>(e)->GetWorldPosition();
		
	}

	auto view = registry.View<FollowCameraComponent>();

	for (auto e : view) {
		auto* transform = registry.GetComponent<No::TransformComponent>(e);
		auto* followCameraVariables = registry.GetComponent<FollowCameraComponent>(e);

		if (No::Keyboard::IsPress(VK_LEFT)) {
			followCameraVariables->theta += 1.f * deltaTime;
		}
		if (No::Keyboard::IsPress(VK_RIGHT)) {
			followCameraVariables->theta -= 1.f * deltaTime;
		}
		if (No::Keyboard::IsPress(VK_UP)) {
			followCameraVariables->phi += 1.f * deltaTime;
			if (followCameraVariables->phi >= PI) {
				followCameraVariables->phi = PI - 0.001f;
			}
		}
		if (No::Keyboard::IsPress(VK_DOWN)) {
			followCameraVariables->phi -= 1.f * deltaTime;
			if (followCameraVariables->phi <= 0.f) {
				followCameraVariables->phi = 0.001f;
			}
		}

		transform->translate.x = playerPos.x + followCameraVariables->distance * std::sin(followCameraVariables->phi) * std::cos(followCameraVariables->theta);
		transform->translate.y = playerPos.y + followCameraVariables->distance * std::cos(followCameraVariables->phi);
		transform->translate.z = playerPos.z + followCameraVariables->distance * std::sin(followCameraVariables->phi) * std::sin(followCameraVariables->theta);

		transform->rotation.LookRotation(playerPos - transform->GetWorldPosition(), No::Vector3::UP);
		
		
	}



}
