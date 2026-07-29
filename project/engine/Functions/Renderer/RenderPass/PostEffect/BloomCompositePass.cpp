#include "stdafx.h"
#include "BloomCompositePass.h"
#include "engine/Functions/Shader/ShaderReflection.h"

namespace NoEngine {
namespace Render {
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