#include "IScene.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"

namespace NoEngine {
namespace Scene {
IScene::IScene() :
	registry_(std::make_unique<ECS::Registry>()),
	systemManager_(std::make_unique<ECS::SystemManager>()) {

	registry_->AddComponent<SceneNameComponent>(registry_->GenerateEntity());
	registry_->AddComponent<ECS::PauseComponent>(registry_->GenerateEntity());

	// すべてのシーンにカメラを配置する
	{
		auto mainCamera = registry_->GenerateEntity();
		registry_->AddComponent<Component::ActiveCameraTag>(mainCamera);
		registry_->AddComponent<Component::CameraComponent>(mainCamera);
		registry_->AddComponent<Component::TransformComponent>(mainCamera);
		registry_->AddComponent<Editor::EditTag>(mainCamera)->name = "MainCamera";
	}

	// すべてのシーンにデバッグカメラを配置する
	{
		auto debugCamera = registry_->GenerateEntity();
		registry_->AddComponent<Component::DebugCameraComponent>(debugCamera);
		registry_->AddComponent<Component::CameraComponent>(debugCamera);
		registry_->AddComponent<Component::TransformComponent>(debugCamera);
		registry_->AddComponent<Editor::EditTag>(debugCamera)->name = "DebugCamera";
	}
}

void IScene::Update(ComputeContext& ctx, float deltaTime) {
	NotSystemUpdate();
	systemManager_->UpdateAll(ctx, *registry_, deltaTime);
	registry_->FlushDestroy();
}

std::string GetCurrentSceneName(ECS::Registry& registry) {
	auto view = registry.View<SceneNameComponent>();
	std::string currentName;
	for (auto entity : view) {
		auto* scene = registry.GetComponent<SceneNameComponent>(entity);
		currentName = scene->GetName();
	}

	return currentName;
}

}
}