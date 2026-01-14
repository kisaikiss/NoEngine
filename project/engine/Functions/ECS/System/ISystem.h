#pragma once
#include "../Registry.h"
namespace NoEngine {
namespace ECS {
class ISystem {
public:
	virtual ~ISystem() = default;
	virtual void Update(ECS::Registry& registry, float deltaTime) = 0;
};
}
}