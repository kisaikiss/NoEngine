#include "stdafx.h"
#include "DepthOfFieldPass.h"
#include "engine/Functions/Shader/ShaderReflection.h"

namespace NoEngine {
namespace Render {

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
