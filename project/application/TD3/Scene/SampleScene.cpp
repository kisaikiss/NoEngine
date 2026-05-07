#include "SampleScene.h"
#include "application/TD3/Utility/GameResourceComponent.h"
#include "application/TD3/Utility/GameAudio.h"
#include "application/TD3/Utility/InputHelperSystem.h"


namespace {
	No::Entity cameraE;
}


void SampleScene::Setup() {
	AddSystem(std::make_unique<InputHelperSystem>());
	AddSystem(std::make_unique<No::DebugCameraSystem>());
	AddSystem(std::make_unique<No::CameraSystem>());

	No::Registry& registry = *GetRegistry();


	auto camera = registry.GenerateEntity();
	registry.AddComponent<No::ActiveCameraTag>(camera);
	registry.AddComponent<No::CameraComponent>(camera);
	registry.AddComponent<No::DebugCameraComponent>(camera);
	auto* cameraEditTag = registry.AddComponent<No::EditTag>(camera);
	cameraEditTag->name = "camera";
	auto* cameraTransform = registry.AddComponent<No::TransformComponent>(camera);
	cameraTransform->translate.z = -5.f;

	cameraE = registry.GenerateEntity();
	registry.AddComponent<No::CameraComponent>(cameraE);
	auto* cameraTransform2 = registry.AddComponent<No::TransformComponent>(cameraE);
	cameraTransform2->translate.z = -5.f;

	auto light = registry.GenerateEntity();
	auto* dir = registry.AddComponent<No::DirectionalLightComponent>(light);
	dir->color = { 1.f,1.f,1.f,1.f };
	dir->direction = { 0.f,-1.f,0.f };
	dir->intensity = 0.1f;
	auto* lightTag = registry.AddComponent<No::EditTag>(light);
	lightTag->name = "directionalLight";
}

void SampleScene::NotSystemUpdate() {
}
