#include "Scene.h"
#include "engine/Functions/ECS/Component/Common/PauseComponent.h"
#include "engine/Functions/ECS/System/SystemSceneConfig.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"

namespace NoEngine::Scene {

Scene::Scene(const std::string& name) : name_(name) {
	registry_.AddComponent<SceneNameComponent>(registry_.GenerateEntity())->name = name;
	registry_.AddComponent<ECS::PauseComponent>(registry_.GenerateEntity());
}

bool Scene::Load() {
	try {
		std::ifstream file(ECS::SystemSceneConfig::GetScenePath(name_));
		if (!file) throw std::runtime_error("Scene file could not be opened");
		nlohmann::json data;
		file >> data;
		if (!data.at("entities").is_object()) throw std::runtime_error("Invalid scene entities");

		const auto names = data.value("systems", std::vector<std::string>{ "EditSystem", "DrawManipulatorSystem" });
		if (!systemManager_.SetSystems(names)) throw std::runtime_error("Invalid scene systems");
		Editor::LoadScene(registry_, data);
		return true;
	} catch (const std::exception& error) {
		LogWarning("Failed to load scene '" + name_ + "': " + error.what());
		return false;
	}
}

void Scene::Update(ComputeContext& ctx, float deltaTime) {
	systemManager_.UpdateAll(ctx, registry_, deltaTime);
	registry_.FlushDestroy();
}

std::string GetCurrentSceneName(ECS::Registry& registry) {
	for (auto entity : registry.View<SceneNameComponent>()) {
		return registry.GetComponent<SceneNameComponent>(entity)->GetName();
	}
	return {};
}

}
