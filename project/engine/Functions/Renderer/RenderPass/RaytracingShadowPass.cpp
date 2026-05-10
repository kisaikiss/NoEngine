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

	gfx.SetRaytracingRootSignature(Render::GetRootSignature(Render::GetRootSignatureID(L"RT Global RootSignature")));
	
	// tlas
	gfx.SetComputeSRV(0, renderContext->GetTLAS()->GetGPUVirtualAddress());
	
	// depth
	gfx.TransitionResource(GraphicsCore::GetDepth(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	gfx.SetRaytracingDynamicDescriptor(1, 0, GraphicsCore::GetDepth().GetDepthSRV());
	
	// lights
	gfx.SetRaytracingDynamicDescriptor(2, 0, renderContext->GetDirectionalLightSRV());

	// shadow Mask UAV
	gfx.TransitionResource(GraphicsCore::GetShadowMask(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	gfx.SetRaytracingDynamicDescriptor(3, 0, GraphicsCore::GetShadowMask().GetUAV());

	// camera
	struct CameraCB {
		Math::Matrix4x4 invViewProj{};
		Math::Vector3 cameraPos{};
		float pad = 0.f;
	};
	CameraCB cameraData;
	cameraData.invViewProj = camera_->forGPU.viewProjection;
	cameraData.invViewProj.Inverse();
	cameraData.invViewProj.Transpose();
	cameraData.cameraPos = camera_->forGPU.worldPosition;

	gfx.SetRaytracingDynamicConstantBufferView(4, sizeof(CameraCB), &cameraData);

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
	gfx.SetRaytracingDynamicConstantBufferView(5, sizeof(constants), &constants);

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> dxrCommandList;
	gfx.GetCommandList()->QueryInterface(IID_PPV_ARGS(&dxrCommandList));
	dxrCommandList->SetPipelineState1(stateObject.Get());

	gfx.DispatchRays(GetShadowDispatchRaysDesc());

	gfx.TransitionResource(GraphicsCore::GetDepth(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
}
}
}

