#include "stdafx.h"
#include "DissolvePass.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#include "engine/Assets/AssetManager.h"
#include "engine/Functions/ECS/Component/Asset/DissolveComponent.h"

namespace NoEngine {
namespace Render {
DissolvePass::DissolvePass() {
}

void DissolvePass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	static_cast<void>(resourceRegistry);
	for (auto e : registry.View<Component::DissolveComponent>()) {
		auto* dissolve = registry.GetComponent<Component::DissolveComponent>(e);
		threshold_ = dissolve->threshold;
		if (!maskTexture_.IsValid()) {
			maskTexture_ = TextureManager::LoadCovertTexture(AssetManager::GetFilePathFromAddressableName(dissolve->maskTextureName));
		}
	}
	if (!maskTexture_.IsValid()) return;
	auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : Dissolve");
	auto renderCtx = GetRenderContext();
	gfx.SetPipelineState(renderCtx->GetGraphicsPSO("Renderer : Dissolve PSO"));
	gfx.SetRootSignature(renderCtx->GetRootSignature("Renderer : Dissolve"));
	gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_declspec(align(16)) struct {
		float threshold = 0.5f;
		float pad[3]{};
	}constants;
	constants.threshold = threshold_;
	gfx.SetDynamicConstantBufferView(rootIndex["gThreshold"], sizeof(constants), &constants);
	gfx.SetDynamicDescriptor(rootIndex["gTexture"], 0, resourceRegistry.GetColorBuffer(GetInput("InputColor")).GetSRV());
	gfx.SetDynamicDescriptor(rootIndex["gMaskTexture"], 0, maskTexture_.GetSRV());
	gfx.Draw(3);
}
}
}
