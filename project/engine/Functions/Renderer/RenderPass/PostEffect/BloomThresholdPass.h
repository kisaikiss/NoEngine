#pragma once
#include "../RenderPass.h"

namespace NoEngine {
namespace Render {
class BloomThresholdPass : public RenderPass {
public:
	void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
private:
	float threshold_ = 1.0f;
	float softKnee_ = 1.0f;
};
}
}