#pragma once
#include "../RenderPass.h"

namespace NoEngine::Render {

class VolumetricFogPass : public RenderPass {
public:
	void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resources, ECS::Registry& registry) override;
	void DrawEditor() override;
	nlohmann::json SaveEditorSettings() const override;
	void LoadEditorSettings(const nlohmann::json& settings) override;

private:
	float density_ = 0.012f;
	float baseHeight_ = 0.0f;
	float heightFalloff_ = 0.035f;
	float maxDistance_ = 140.0f;
	float ambient_ = 0.15f;
	float scatteringAlbedo_ = 0.85f;
	float scatteringAnisotropy_ = 0.0f;
	int stepCount_ = 16;
	bool traceShadows_ = true;
};

class VolumetricFogCompositePass : public RenderPass {
public:
	void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resources, ECS::Registry& registry) override;
};

}
