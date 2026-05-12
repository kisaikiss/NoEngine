#pragma once
#include "ISystem.h"

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


private:
	std::vector<std::unique_ptr<ISystem>> systems_;

	bool gameStop_ = false;
};
}
}