#include "stdafx.h"
#include "BloomCompositePass.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

namespace NoEngine {
namespace Render {
void BloomCompositePass::DrawEditor() {
	RenderPass::DrawEditor();
#ifdef USE_IMGUI
    ImGui::DragFloat("Bloom intensity", &intensity_, 0.05f, 0.0f, 20.0f);
#endif
}
nlohmann::json BloomCompositePass::SaveEditorSettings() const {
    auto settings = RenderPass::SaveEditorSettings();
    settings["intensity"] = intensity_;
    return settings;
}
void BloomCompositePass::LoadEditorSettings(const nlohmann::json& settings) {
    RenderPass::LoadEditorSettings(settings);
    if (settings.is_object() && settings.contains("intensity") && settings["intensity"].is_number()) intensity_ = settings["intensity"].get<float>();
}
void BloomCompositePass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	static_cast<void>(registry);

	auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : BloomComposite");
	auto renderCtx = GetRenderContext();
	gfx.SetPipelineState(renderCtx->GetGraphicsPSO("Renderer : BloomComposite PSO"));
	gfx.SetRootSignature(renderCtx->GetRootSignature("Renderer : BloomComposite"));
	gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_declspec(align(16)) struct {
		float intensity;
		float pad[3];
	} constants{ intensity_, {} };
	gfx.SetDynamicConstantBufferView(rootIndex["gComposite"], sizeof(constants), &constants);

	gfx.SetDynamicDescriptor(rootIndex["gSceneColor"], 0, resourceRegistry.GetColorBuffer(GetInput("SceneColor")).GetSRV());
	gfx.SetDynamicDescriptor(rootIndex["gBloomColor"], 0, resourceRegistry.GetColorBuffer(GetInput("BloomColor")).GetSRV());
	gfx.Draw(3);
}
}
}
