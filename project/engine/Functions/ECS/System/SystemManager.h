#pragma once
#include "ISystem.h"
#include "externals/nlohmann/json.hpp"

namespace NoEngine {
namespace ECS {
class SystemManager {
public:
	SystemManager();

	template<typename T>
	T* AddSystem(std::unique_ptr<T> system) {
		T* ptr = system.get();
		systems_.push_back(std::move(system));
		return ptr;
	}

	void UpdateAll(ComputeContext& ctx, Registry& registry, float deltaTime);

	// Editing以外(Playing/Paused)ならtrue。">"を押してから"■"を押すまでの間ずっとtrue。
	static bool IsInPlayMode();
	// ">"を押した瞬間(Editing→Playing)に取得したシーンのスナップショット
	static const nlohmann::json& GetPlaySnapshot();

private:
	std::vector<std::unique_ptr<ISystem>> systems_;
};
}
}