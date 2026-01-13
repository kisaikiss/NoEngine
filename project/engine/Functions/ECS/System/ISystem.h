#pragma once
#include "../Registry.h"
namespace NoEngine {
namespace System {
class ISystem {
public:
	virtual ~ISystem() = default;
	virtual void Update(ECS::Registry& registry, float deltaTime) = 0;
};
}
}