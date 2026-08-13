#pragma once
#include "../ISystem.h"
namespace NoEngine {
namespace ECS {
class ComputePlatformDeltaSystem :
	public ISystem {
public:
	void Update(Registry& registry, float deltaTime) override;
};
}
}