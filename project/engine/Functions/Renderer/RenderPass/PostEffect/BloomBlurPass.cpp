#include "stdafx.h"
#include "BloomBlurPass.h"
#include "engine/Functions/Shader/ShaderReflection.h"

namespace NoEngine {
namespace Render {
void BloomBlurPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	static_cast<void>(registry);

	auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : BloomBlur");
	auto renderCtx = GetRenderContext();
	gfx.SetPipelineState(renderCtx->GetGraphicsPSO("Renderer : BloomBlur PSO"));
	gfx.SetRootSignature(renderCtx->GetRootSignature("Renderer : BloomBlur"));
	gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto& srcBuffer = resourceRegistry.GetColorBuffer(GetInput("InputColor"));

	_declspec(align(16)) struct {
		float texelSizeX;
		float texelSizeY;
		float directionX;
		float directionY;
	} constants{
		1.0f / srcBuffer.GetWidth(),
		1.0f / srcBuffer.GetHeight(),
		horizontal_ ? 1.0f : 0.0f,
		horizontal_ ? 0.0f : 1.0f
	};
	gfx.SetDynamicConstantBufferView(rootIndex["gBlur"], sizeof(constants), &constants);
	gfx.SetDynamicDescriptor(rootIndex["gTexture"], 0, srcBuffer.GetSRV());
	gfx.Draw(3);
}
}
}