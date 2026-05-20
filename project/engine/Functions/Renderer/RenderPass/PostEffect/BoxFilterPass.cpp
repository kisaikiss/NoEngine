#include "stdafx.h"
#include "BoxFilterPass.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Functions/Shader/ShaderReflection.h"

namespace NoEngine {
namespace Render {
void BoxFilterPass::Execute(GraphicsContext& gfx, ECS::Registry& registry) {
	static_cast<void>(registry);
	gfx.SetRenderTarget(GraphicsCore::sFinalColorBuffer.GetRTV());
	gfx.TransitionResource(GraphicsCore::GetPostEffectBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

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

	// ToDo: ApplicationでKernelSizeを設定できるようにする
	constants.kernelSize = 5;
	int count = constants.kernelSize * constants.kernelSize;
	float v = 1.0f / count;
	for (int i = 0; i < count; ++i) constants.kernelVec[i] = v;
	// 残りは 0 にしておく
	for (int i = count; i < KMaxKernelVec * 4; ++i) constants.kernelVec[i] = 0.0f;

	gfx.SetDynamicConstantBufferView(rootIndex["KernelCB"], sizeof(constants), &constants);
	gfx.SetDynamicDescriptor(rootIndex["gTexture"], 0, GraphicsCore::GetPostEffectBuffer().GetSRV());
	gfx.Draw(3);
}
}
}

