#include "stdafx.h"
#include "Camera2DChangeSystem.h"

#include "../Component/FollowCamera2DComponent.h"

void Camera2DChangeSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
#ifdef USE_IMGUI
	auto cameraView = registry.View<FollowCamera2DComponent>();
	auto debugCameraView = registry.View<No::DebugCamera2DComponent>();
	bool isChangeCamera = false;

	if (No::Keyboard::IsTrigger('1')) {
		isChangeCamera = true;
	}
	

	if (isChangeCamera) {
		for (auto e : cameraView) {
			if (!registry.Has<No::ActiveCamera2DTag>(e)) {
				registry.AddComponent<No::ActiveCamera2DTag>(e);
				return;
			}
		}
		for (auto e : debugCameraView) {
			if (!registry.Has<No::ActiveCamera2DTag>(e)) {
				registry.AddComponent<No::ActiveCamera2DTag>(e);
				return;
			}
		}
	}
#else
	static_cast<void>(registry);
#endif // USE_IMGUI

	

}
