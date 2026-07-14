#include "stdafx.h"
#include "DebugCamera2DSystem.h"
#include "../../Component/CameraComponent.h"
#include "../../Component/Transform2DComponent.h"
#include "engine/Functions/Input/Input.h"
#include "../../../../Editor/EditUtils.h"

namespace NoEngine {
namespace ECS {

void DebugCamera2DSystem::Update(Registry& registry, float deltaTime) {
	if (!Editor::IsMouseOverSceneWindow()) return;
	auto view = registry.View<Component::DebugCamera2DComponent, Component::Camera2DComponent, Component::Transform2DComponent>();
	for (auto e : view) {
		auto* transform = registry.GetComponent<Component::Transform2DComponent>(e);
		auto* debugCamera = registry.GetComponent<Component::DebugCamera2DComponent>(e);
		
		// 移動
		debugCamera->preMousePositionX = debugCamera->mousePositionX;
		debugCamera->preMousePositionY = debugCamera->mousePositionY;
		if (Input::Mouse::IsPress(Input::MouseButton::Middle)) {
			if (Input::Mouse::IsTrigger(Input::MouseButton::Middle)) {
				debugCamera->mousePositionX = 0;
				debugCamera->mousePositionY = 0;
				debugCamera->preMousePositionX = 0;
				debugCamera->preMousePositionY = 0;
			} else {

				Input::Mouse::GetPosition(&debugCamera->mousePositionX, &debugCamera->mousePositionY);
			}
		}

		if (debugCamera->preMousePositionX && debugCamera->mousePositionX) {
			Math::Vector2 velocity{};

			static const float kCameraSpeed = 100.f;
			
			float cameraSpeedRevision = transform->scale.x;
			velocity.x = (static_cast<float>(debugCamera->preMousePositionX - debugCamera->mousePositionX)) * kCameraSpeed * cameraSpeedRevision;
			velocity.y = (static_cast<float>(debugCamera->preMousePositionY - debugCamera->mousePositionY)) * kCameraSpeed * cameraSpeedRevision;


			transform->translate += velocity * deltaTime;
		}


		// 拡縮
		debugCamera->preMouseWheelY = debugCamera->mouseWheelY;
		debugCamera->mouseWheelY = Input::Mouse::GetMouseWheelY();

		float deltaMouseWheel = debugCamera->preMouseWheelY - debugCamera->mouseWheelY;
		if (std::fabs(deltaMouseWheel) < 140.f) {

			float cameraSpeedRevision = transform->scale.x / 2.f;
			transform->scale += ((deltaMouseWheel) / 500.0f) * cameraSpeedRevision;
			if (transform->scale.x < 0.6f) {
				transform->scale = 0.6f;
			}
		}
	}
}

}
}
