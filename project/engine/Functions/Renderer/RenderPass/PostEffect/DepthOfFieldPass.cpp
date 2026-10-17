#include "stdafx.h"
#include "DepthOfFieldPass.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

namespace NoEngine {
namespace Render {

void DepthOfFieldPass::DrawEditor() {
	RenderPass::DrawEditor();
#ifdef USE_IMGUI
	ImGui::DragFloat("Focus distance", &focusDistance_, 0.5f, 0.1f, 1000.0f);
	ImGui::DragFloat("Focus range", &focusRange_, 0.5f, 0.1f, 1000.0f);
	ImGui::DragFloat("Max CoC radius", &maxCoCRadius_, 0.001f, 0.0f, 0.25f, "%.3f");
#endif
}

nlohmann::json DepthOfFieldPass::SaveEditorSettings() const {
	auto settings = RenderPass::SaveEditorSettings();
	settings["focusDistance"] = focusDistance_;
	settings["focusRange"] = focusRange_;
	settings["maxCoCRadius"] = maxCoCRadius_;
	return settings;
}

void DepthOfFieldPass::LoadEditorSettings(const nlohmann::json& settings) {
	if (!settings.is_object()) return;
	RenderPass::LoadEditorSettings(settings);
	if (settings.contains("focusDistance") && settings["focusDistance"].is_number()) focusDistance_ = settings["focusDistance"].get<float>();
	if (settings.contains("focusRange") && settings["focusRange"].is_number()) focusRange_ = settings["focusRange"].get<float>();
	if (settings.contains("maxCoCRadius") && settings["maxCoCRadius"].is_number()) maxCoCRadius_ = settings["maxCoCRadius"].get<float>();
}

void DepthOfFieldPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	static_cast<void>(registry);

	auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : DepthOfField");
	auto renderCtx = GetRenderContext();
	gfx.SetPipelineState(renderCtx->GetGraphicsPSO("Renderer : DepthOfField PSO"));
	gfx.SetRootSignature(renderCtx->GetRootSignature("Renderer : DepthOfField"));
	gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.SetDynamicDescriptor(rootIndex["gTexture"], 0, resourceRegistry.GetColorBuffer(GetInput("InputColor")).GetSRV());
	gfx.SetDynamicDescriptor(rootIndex["gDepthTexture"], 0, resourceRegistry.GetDepthBuffer(GetInput("InputDepth")).GetDepthSRV());

	Math::Matrix4x4 projection = GetTargetCamera()->projection;
	projection.Inverse();

	_declspec(align(16)) struct {
		Math::Matrix4x4 projectionInverse;
		float focusDistance;
		float focusRange;
		float maxCoCRadius;
		float padding;
	} constants{ projection, focusDistance_, focusRange_, maxCoCRadius_, 0.0f };

	gfx.SetDynamicConstantBufferView(rootIndex["gMaterial"], sizeof(constants), &constants);
	gfx.Draw(3);
}

}
}
