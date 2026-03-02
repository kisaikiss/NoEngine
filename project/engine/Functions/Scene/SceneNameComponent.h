#pragma once
namespace NoEngine {
namespace Scene { class IScene; }
struct SceneNameComponent {
	const std::string& GetName() const { return name; }
private:
	friend class Scene::IScene;
	std::string name;
};
}