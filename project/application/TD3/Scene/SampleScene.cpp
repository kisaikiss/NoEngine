#include "SampleScene.h"

void SampleScene::Setup() {
	AddSystem(std::make_unique<No::CameraSystem>());
	No::Registry& registry = *GetRegistry();
	auto camera = registry.GenerateEntity();
	registry.AddComponent<No::ActiveCameraTag>(camera);
	registry.AddComponent<No::CameraComponent>(camera);
	auto* cameraTransform = registry.AddComponent<No::TransformComponent>(camera);
	cameraTransform->translate.z = -5.f;
}

void SampleScene::NotSystemUpdate() {
}
