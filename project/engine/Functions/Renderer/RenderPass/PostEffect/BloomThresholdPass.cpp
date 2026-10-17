#include "stdafx.h"
#include "BloomThresholdPass.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

namespace NoEngine {
namespace Render {
void BloomThresholdPass::DrawEditor() {
	RenderPass::DrawEditor();
#ifdef USE_IMGUI
	ImGui::DragFloat("Threshold", &threshold_, 0.05f, 0.0f, 20.0f);
	ImGui::DragFloat("Soft knee", &softKnee_, 0.05f, 0.0f, 5.0f);
#endif
}
nlohmann::json BloomThresholdPass::SaveEditorSettings() const {
	auto settings = RenderPass::SaveEditorSettings();
	settings["threshold"] = threshold_;
	settings["softKnee"] = softKnee_;
	return settings;
}
void BloomThresholdPass::LoadEditorSettings(const nlohmann::json& settings) {
	if (!settings.is_object()) return;
	RenderPass::LoadEditorSettings(settings);
	if (settings.contains("threshold") && settings["threshold"].is_number()) threshold_ = settings["threshold"].get<float>();
	if (settings.contains("softKnee") && settings["softKnee"].is_number()) softKnee_ = settings["softKnee"].get<float>();
}
void BloomThresholdPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	static_cast<void>(registry);

	auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : BloomThreshold");
	auto renderCtx = GetRenderContext();
	gfx.SetPipelineState(renderCtx->GetGraphicsPSO("Renderer : BloomThreshold PSO"));
	gfx.SetRootSignature(renderCtx->GetRootSignature("Renderer : BloomThreshold"));
	gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_declspec(align(16)) struct {
		float threshold;
		float softKnee;
		float pad[2];
	} constants{ threshold_, softKnee_, {} };
	gfx.SetDynamicConstantBufferView(rootIndex["gThreshold"], sizeof(constants), &constants);

	gfx.SetDynamicDescriptor(rootIndex["gTexture"], 0, resourceRegistry.GetColorBuffer(GetInput("InputColor")).GetSRV());
	gfx.Draw(3);
}
}
}
