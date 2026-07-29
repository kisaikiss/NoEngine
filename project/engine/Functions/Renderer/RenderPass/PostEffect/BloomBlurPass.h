#pragma once
#include "../RenderPass.h"

namespace NoEngine {
namespace Render {
class BloomBlurPass : public RenderPass {
public:
    explicit BloomBlurPass(bool horizontal) : horizontal_(horizontal) {}
    void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
private:
    bool horizontal_;
};
}
}