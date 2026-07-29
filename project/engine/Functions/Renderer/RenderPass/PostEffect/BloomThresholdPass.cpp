#include "stdafx.h"
#include "BloomThresholdPass.h"
#include "engine/Functions/Shader/ShaderReflection.h"

namespace NoEngine {
namespace Render {
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