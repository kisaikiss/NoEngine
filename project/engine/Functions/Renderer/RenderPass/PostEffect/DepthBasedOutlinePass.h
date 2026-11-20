#pragma once
#include "../RenderPass.h"

namespace NoEngine {
namespace Render {
class DepthBasedOutlinePass : public RenderPass {
public:
	void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
	void DrawEditor() override;
	nlohmann::json SaveEditorSettings() const override;
	void LoadEditorSettings(const nlohmann::json& settings) override;
private:
	bool chalkEnabled_ = false;
	float chalkWidth_ = 1.6f;
	float chalkThreshold_ = 0.015f;
	float chalkRoughness_ = 0.65f;
	float chalkSurfaceBrightness_ = 0.35f;
	Math::Vector3 chalkColor_ = { 0.9f, 0.92f, 0.82f };
};
}
}
