#include "RaytracingShadowPass.h"

#include "../../RenderSystem.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Functions/ECS/Component/Asset/LightComponent.h"

namespace NoEngine {
namespace Render {

void RaytracingShadowPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	static_cast<void>(resourceRegistry);
	Collect(registry);
	Dispatch(gfx, resourceRegistry);
}

void RaytracingShadowPass::Collect(ECS::Registry& registry) {
	
	camera_ = GetTargetCamera();

	auto lightView = registry.View<Component::DirectionalLightComponent>();
	for (auto entity : lightView) {
		lightDir_ = registry.GetComponent<Component::DirectionalLightComponent>(entity)->direction;

	}

}

void RaytracingShadowPass::Dispatch(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry) {
	auto& stateObject = Render::GetShadowRtStateObject();
	auto* renderContext = GetRenderContext();

	auto* lightNums = renderContext->GetLightNums();
	if (!renderContext->GetTLAS().Get() || !camera_) return;
	if (lightNums->directionalLightNum == 0 &&
		lightNums->pointLightNum == 0 &&
		lightNums->spotLightNum == 0) return;

	gfx.SetRaytracingRootSignature(Render::GetRootSignature(Render::GetRootSignatureID(L"RT Global RootSignature")));

	gfx.SetComputeSRV(0, renderContext->GetTLAS()->GetGPUVirtualAddress());
	gfx.SetRaytracingDynamicDescriptor(1, 0, resourceRegistry.GetColorBuffer("WorldPosition").GetSRV());
	gfx.SetRaytracingDynamicDescriptor(6, 0, resourceRegistry.GetColorBuffer("Normal").GetSRV());

	if (lightNums->directionalLightNum) {
		gfx.SetRaytracingDynamicDescriptor(2, 0, renderContext->GetDirectionalLightSRV());
	}
	if (lightNums->pointLightNum) {
		gfx.SetRaytracingDynamicDescriptor(7, 0, renderContext->GetPointLightSRV());
	}
	if (lightNums->spotLightNum) {
		gfx.SetRaytracingDynamicDescriptor(8, 0, renderContext->GetSpotLightSRV());
	}

	gfx.SetRaytracingDynamicDescriptor(3, 0, resourceRegistry.GetColorBuffer("ShadowMask").GetUAV());


	// camera
	struct CameraCB {
		Math::Matrix4x4 invViewProj{};
		Math::Vector3 cameraPos{};
		float pad1 = 0.f;
		Math::Vector3 lightDir{};
		float pad2 = 0.f;
	};
	CameraCB cameraData;
	cameraData.invViewProj = camera_->forGPU.viewProjection;
	cameraData.invViewProj.Inverse();
	cameraData.invViewProj.Transpose();
	cameraData.cameraPos = camera_->forGPU.worldPosition;
	cameraData.lightDir = -lightDir_;

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

}
}
}

