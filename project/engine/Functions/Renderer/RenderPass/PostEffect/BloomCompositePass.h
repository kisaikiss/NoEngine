#pragma once
#include "../RenderPass.h"

namespace NoEngine {
namespace Render {
class BloomCompositePass : public RenderPass {
public:
    void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
    void DrawEditor() override;
    nlohmann::json SaveEditorSettings() const override;
    void LoadEditorSettings(const nlohmann::json& settings) override;
private:
    float intensity_ = 3.f;
};
}
}
