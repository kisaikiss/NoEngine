#include "stdafx.h"
#include "RandomNoisePass.h"
#include "engine/Runtime/GameCore.h"
#include "engine/Functions/Shader/ShaderReflection.h"

namespace NoEngine {
namespace Render {
void RandomNoisePass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	static_cast<void>(resourceRegistry);
	static_cast<void>(registry);

	auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : RandomNoise");
	auto renderCtx = GetRenderContext();
	gfx.SetPipelineState(renderCtx->GetGraphicsPSO("Renderer : RandomNoise PSO"));
	gfx.SetRootSignature(renderCtx->GetRootSignature("Renderer : RandomNoise"));
	gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_declspec(align(16)) struct {
		float time;
		float padding[3];
	}constants;
	constants.time = GameCore::GetElapsedTime();
	gfx.SetDynamicConstantBufferView(rootIndex["SeedCB"], sizeof(constants), &constants);
	gfx.SetDynamicDescriptor(rootIndex["gTexture"], 0, resourceRegistry.GetColorBuffer(GetInput("InputColor")).GetSRV());
	gfx.Draw(3);
}
}
}