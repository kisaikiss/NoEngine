#pragma once
#include "engine/Runtime/Command/ComputeContext.h"
#include "../Registry.h"
namespace NoEngine {
namespace ECS {
class ISystem {
public:
	virtual ~ISystem() = default;
	virtual void Update(ComputeContext& ctx, ECS::Registry& registry, float deltaTime) {
		static_cast<void>(ctx);
		Update(registry, deltaTime);
	}
	virtual void Update(ECS::Registry& registry, float deltaTime) = 0;
};
}
}