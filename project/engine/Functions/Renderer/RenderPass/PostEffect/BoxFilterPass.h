#pragma once
#include "../RenderPass.h"

namespace NoEngine {
namespace Render {
class BoxFilterPass : public RenderPass {
public:
	void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
	void DrawEditor() override;
	nlohmann::json SaveEditorSettings() const override;
	void LoadEditorSettings(const nlohmann::json& settings) override;
private:
	uint32_t kernelSize_ = 5;
};
}
}
