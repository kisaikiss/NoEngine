#pragma once
namespace NoEngine {
namespace Scene { class Scene; }
struct SceneNameComponent {
	const std::string& GetName() const { return name; }
private:
	friend class Scene::Scene;
	std::string name;
};
namespace ECS { class Registry; }
namespace Scene { std::string GetCurrentSceneName(ECS::Registry& registry); }
}
