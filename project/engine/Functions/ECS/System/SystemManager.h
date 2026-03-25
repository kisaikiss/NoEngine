#pragma once
#include "ISystem.h"

namespace NoEngine {
namespace ECS {
class SystemManager {
public:
	void AddSystem(std::unique_ptr<ISystem> system) { systems_.push_back(std::move(system)); }

	void UpdateAll(ComputeContext& ctx, Registry& registry, float deltaTime);


private:
	std::vector<std::unique_ptr<ISystem>> systems_;

	bool gameStop_ = false;
};
}
}