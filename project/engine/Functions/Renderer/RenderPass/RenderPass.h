#pragma once
#include "engine/Runtime/Command/GraphicsContext.h"
#include "engine/Functions/ECS/Registry.h"

namespace NoEngine {
namespace Render {
class RenderPass {
public:
	virtual ~RenderPass() = default;
	virtual void Execute(GraphicsContext& gfx, ECS::Registry& registry) = 0;
};
}
}