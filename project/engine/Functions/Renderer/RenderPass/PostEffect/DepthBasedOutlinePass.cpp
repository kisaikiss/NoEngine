#include "stdafx.h"
#include "DepthBasedOutlinePass.h"
#include "engine/Functions/Shader/ShaderReflection.h"

namespace NoEngine {
namespace Render {
void DepthBasedOutlinePass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	static_cast<void>(registry);

	auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : Outline");
	auto renderCtx = GetRenderContext();
	gfx.SetPipelineState(renderCtx->GetGraphicsPSO("Renderer : Outline PSO"));
	gfx.SetRootSignature(renderCtx->GetRootSignature("Renderer : Outline"));
	gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.SetDynamicDescriptor(rootIndex["gTexture"], 0, resourceRegistry.GetColorBuffer(GetInput("InputColor")).GetSRV());
	gfx.SetDynamicDescriptor(rootIndex["gDepthTexture"], 0, resourceRegistry.GetDepthBuffer(GetInput("InputDepth")).GetDepthSRV());
	auto projection = GetTargetCamera()->projection;
	projection.Inverse();

	gfx.SetDynamicConstantBufferView(rootIndex["gMaterial"], sizeof(projection), &projection);
	gfx.Draw(3);
}
}
}