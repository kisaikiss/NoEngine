#include "RaytracingShadowPass.h"

#include "../RenderSystem.h"
#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine {
namespace Render {

void RaytracingShadowPass::Execute(GraphicsContext& gfx, ECS::Registry& registry) {
	Collect(registry);
	Dispatch(gfx);
}

void RaytracingShadowPass::Collect(ECS::Registry& registry) {
	auto view = registry.View<Component::ActiveCameraTag, Component::CameraComponent>();

	for (auto entity : view) {
		camera_ = registry.GetComponent<Component::CameraComponent>(entity);
	}

}

void RaytracingShadowPass::Dispatch(GraphicsContext& gfx) {
	auto& stateObject = Render::GetShadowRtStateObject();
	auto* renderContext = GetRenderContext();

	gfx.SetStateObject(stateObject.Get());
	gfx.SetRaytracingRootSignature(Render::GetRootSignature(Render::GetRootSignatureID(L"RT Global RootSignature")));

	
	// tlas
	
	
	// depth
	gfx.TransitionResource(GraphicsCore::GetDepth(), D3D12_RESOURCE_STATE_DEPTH_READ);
	gfx.SetDynamicDescriptor(1, 0, GraphicsCore::GetDepth().GetDepthSRV());
	
	// lights
	gfx.SetDynamicDescriptor(2, 0, renderContext->GetDirectionalLightSRV());

	// shadow Mask UAV
	gfx.SetDynamicDescriptor(3, 0, GraphicsCore::GetShadowMask().GetSRV());

	// camera
	gfx.SetDynamicConstantBufferView(4, sizeof(Component::CameraForGPU), &camera_->forGPU);

	// lightNum
	_declspec(align(16)) struct {
		uint32_t directionalLightNum = 0;
		uint32_t pointLightNum = 0;
		uint32_t spotLightNum = 0;
		uint32_t pad = 0;
	}constants;
	constants.directionalLightNum = GetRenderContext()->GetLightNums()->directionalLightNum;
	constants.pointLightNum = GetRenderContext()->GetLightNums()->pointLightNum;
	constants.spotLightNum = GetRenderContext()->GetLightNums()->spotLightNum;
	gfx.SetDynamicConstantBufferView(5, sizeof(constants), &constants);

	gfx.DispatchRays(GetShadowDispatchRaysDesc());
}
}
}

