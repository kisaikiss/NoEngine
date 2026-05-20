#include "stdafx.h"
#include "VignettingPass.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Functions/Shader/ShaderReflection.h"

namespace NoEngine {
namespace Render {
void VignettingPass::Execute(GraphicsContext& gfx, ECS::Registry& registry) {
	static_cast<void>(registry);
	gfx.SetRenderTarget(GraphicsCore::sFinalColorBuffer.GetRTV());
	gfx.TransitionResource(GraphicsCore::GetPostEffectBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : PostEffectRootSig");
	auto renderCtx = GetRenderContext();
	gfx.SetPipelineState(renderCtx->GetGraphicsPSO("Renderer : Vignetting PSO"));
	gfx.SetRootSignature(renderCtx->GetRootSignature("Renderer : PostEffectRootSig"));
	gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.SetDynamicDescriptor(rootIndex["gTexture"], 0, GraphicsCore::GetPostEffectBuffer().GetSRV());
	gfx.Draw(3);
}
}
}
