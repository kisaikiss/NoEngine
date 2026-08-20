#pragma once
#include "../RenderPass.h"

namespace NoEngine {
namespace Render {
class BloomCompositePass : public RenderPass {
public:
    void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
private:
    float intensity_ = 3.f;
};
}
}