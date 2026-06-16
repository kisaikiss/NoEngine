#pragma once
#include "../RenderPass.h"

namespace NoEngine {
namespace Render {
class DepthBasedOutlinePass : public RenderPass {
public:
	void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
};
}
}