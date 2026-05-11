#include "stdafx.h"
#include "RaytracingTestPass.h"

#include "../../RenderSystem.h"
#include "engine/Runtime/GraphicsCore.h"
#include "../../../ECS/Component/TransformComponent.h"

namespace NoEngine {
namespace Render {
void RaytracingTestPass::Execute(GraphicsContext& gfx, ECS::Registry& registry) {
	auto view = registry.View<Component::ActiveCameraTag, Component::CameraComponent>();

	for (auto entity : view) {
		camera_ = registry.GetComponent<Component::CameraComponent>(entity);
		auto* t = registry.GetComponent<Component::TransformComponent>(entity);
		cameraRotate_ = t->rotation;
	}

	auto& stateObject = Render::GetRtStateObject();
	auto* renderContext = GetRenderContext();

	gfx.SetRaytracingRootSignature(Render::GetRootSignature(Render::GetRootSignatureID(L"RT Test Global RootSignature")));

	// tlas
	gfx.SetComputeSRV(0, renderContext->GetTLAS()->GetGPUVirtualAddress());

	// shadow Mask UAV
	gfx.TransitionResource(GraphicsCore::GetRaytracingBuffer(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	gfx.SetRaytracingDynamicDescriptor(1, 0, GraphicsCore::GetRaytracingBuffer().GetUAV());

	// camera
	_declspec(align(16)) struct {
		Math::Matrix4x4 invViewProj{};
		Math::Vector3 cameraPos{};
		float pad = {};
	}cameraData;
	cameraData.invViewProj = camera_->forGPU.viewProjection;
	cameraData.invViewProj.Inverse();
	cameraData.invViewProj.Transpose();
	cameraData.cameraPos = camera_->forGPU.worldPosition;

	gfx.SetRaytracingDynamicConstantBufferView(2, sizeof(cameraData), &cameraData);

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> dxrCommandList;
	gfx.GetCommandList()->QueryInterface(IID_PPV_ARGS(&dxrCommandList));
	dxrCommandList->SetPipelineState1(stateObject.Get());

	gfx.DispatchRays(GetDispatchRaysDesc());
}
}
}