#pragma once
#include "ISystem.h"

namespace NoEngine {
namespace ECS {
class SystemManager {
public:
	template<typename T, typename ... Args>
	T* AddSystem(Args&& ... args) {
		auto sys = std::make_unique<T>(std::forward<Args>(args)...);
		T* ptr = sys.get();
		systems_.push_back(std::move(sys));
		return ptr;
	}

	void UpdateAll(Registry& registry, float deltaTime) {
		for (auto& system : systems_) {
			system->Update(registry, deltaTime);
		}
	}


private:
	std::vector<std::unique_ptr<ISystem>> systems_;
};
}
}