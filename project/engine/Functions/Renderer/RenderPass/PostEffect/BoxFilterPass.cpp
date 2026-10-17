#include "stdafx.h"
#include "BoxFilterPass.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

namespace NoEngine {
namespace Render {
void BoxFilterPass::DrawEditor() {
	RenderPass::DrawEditor();
#ifdef USE_IMGUI
	int kernelSize = static_cast<int>(kernelSize_);
	if (ImGui::SliderInt("Kernel size", &kernelSize, 1, 7)) kernelSize_ = static_cast<uint32_t>(kernelSize);
#endif
}
nlohmann::json BoxFilterPass::SaveEditorSettings() const {
	auto settings = RenderPass::SaveEditorSettings();
	settings["kernelSize"] = kernelSize_;
	return settings;
}
void BoxFilterPass::LoadEditorSettings(const nlohmann::json& settings) {
	RenderPass::LoadEditorSettings(settings);
	if (settings.is_object() && settings.contains("kernelSize") && settings["kernelSize"].is_number_integer()) {
		const int value = settings["kernelSize"].get<int>();
		if (value >= 1 && value <= 7) kernelSize_ = static_cast<uint32_t>(value);
	}
}
void BoxFilterPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	static_cast<void>(resourceRegistry);
	static_cast<void>(registry);

	auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : BoxFilter");
	auto renderCtx = GetRenderContext();
	gfx.SetPipelineState(renderCtx->GetGraphicsPSO("Renderer : BoxFilter PSO"));
	gfx.SetRootSignature(renderCtx->GetRootSignature("Renderer : BoxFilter"));
	gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	static const int KMaxKernel = 49;
	static const int KMaxKernelVec = (KMaxKernel + 3) / 4;
	_declspec(align(16)) struct {
		uint32_t kernelSize;
		uint32_t padding[3];
		float kernelVec[KMaxKernelVec * 4];
	}constants;

	constants.kernelSize = kernelSize_;
	int count = constants.kernelSize * constants.kernelSize;
	float v = 1.0f / count;
	for (int i = 0; i < count; ++i) constants.kernelVec[i] = v;
	// 残りは 0 にしておく
	for (int i = count; i < KMaxKernelVec * 4; ++i) constants.kernelVec[i] = 0.0f;

	gfx.SetDynamicConstantBufferView(rootIndex["KernelCB"], sizeof(constants), &constants);
	gfx.SetDynamicDescriptor(rootIndex["gTexture"], 0, resourceRegistry.GetColorBuffer(GetInput("InputColor")).GetSRV());
	gfx.Draw(3);
}
}
}

