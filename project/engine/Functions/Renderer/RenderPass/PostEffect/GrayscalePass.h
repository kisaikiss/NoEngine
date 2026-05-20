#pragma once
#include "../RenderPass.h"

namespace NoEngine {
namespace Render {
class GrayscalePass : public RenderPass {
public:
	void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
};
}
}
