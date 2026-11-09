#pragma once
#include "../RenderPass.h"
namespace NoEngine {
namespace Render {
class RaytracingShadowPass : public RenderPass {
public:
	void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
private:
	void Dispatch(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry);
};
}
}

