#include "IScene.h"

namespace NoEngine {
namespace Scene {
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