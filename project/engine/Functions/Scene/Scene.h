#pragma once
#include "engine/Functions/ECS/Registry.h"
#include "engine/Functions/ECS/System/SystemManager.h"
#include "SceneNameComponent.h"

namespace NoEngine::Scene {

// JSONに保存された配置とシステム構成を所有する。
class Scene final {
public:
	explicit Scene(const std::string& name);

	void Update(ComputeContext& ctx, float deltaTime);
	ECS::Registry& GetRegistry() { return registry_; }
	const ECS::Registry& GetRegistry() const { return registry_; }
	const std::string& GetName() const { return name_; }
	std::vector<std::string> GetSystemNames() const { return systemManager_.GetSystemNames(); }

private:
	friend class SceneManager;
	bool Load();

	const std::string name_;
	ECS::Registry registry_;
	ECS::SystemManager systemManager_;
};

}
