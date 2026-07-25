#include "stdafx.h"
#include "RaytracingTestPass.h"

#include "../../RaytracingManager.h"
#include "engine/Runtime/GraphicsCore.h"
#include "../../../ECS/Component/Common/TransformComponent.h"

namespace NoEngine {
namespace Render {
void RaytracingTestPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	static_cast<void>(resourceRegistry);
	auto view = registry.View<Component::ActiveCameraTag, Component::CameraComponent>();

	for (auto entity : view) {
		camera_ = registry.GetComponent<Component::CameraComponent>(entity);
		auto* t = registry.GetComponent<Component::TransformComponent>(entity);
		cameraRotate_ = t->rotation;
	}

	auto& stateObject = Render::GetRtStateObject();
	auto* renderContext = GetRenderContext();
	if (!renderContext->GetTLAS().Get()) return;
	gfx.SetRaytracingRootSignature(Render::GetRootSignature(Render::GetRootSignatureID(L"RT Test Global RootSignature")));

	// tlas
	gfx.SetComputeSRV(0, renderContext->GetTLAS()->GetGPUVirtualAddress());

	// UAV
	//gfx.TransitionResource(GraphicsCore::GetRaytracingBuffer(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	//gfx.SetRaytracingDynamicDescriptor(1, 0, GraphicsCore::GetRaytracingBuffer().GetUAV());

	// camera
	_declspec(align(16)) struct {
		Math::Vector3 cameraPos{};
		float pad0;
		Math::Vector3 cameraForward{}; // カメラの正面方向ベクトル
		float pad1;
		Math::Vector3 cameraRight{};   // カメラの右方向ベクトル
		float pad2;
		Math::Vector3 cameraUp{};      // カメラの上方向ベクトル
		float pad3;
	} cameraData;
	cameraData.cameraForward = cameraRotate_.zAxis();
	cameraData.cameraRight = cameraRotate_.xAxis();
	cameraData.cameraUp = cameraRotate_.yAxis();
	cameraData.cameraPos = camera_->forGPU.worldPosition;

	gfx.SetRaytracingDynamicConstantBufferView(2, sizeof(cameraData), &cameraData);

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> dxrCommandList;
	gfx.GetCommandList()->QueryInterface(IID_PPV_ARGS(&dxrCommandList));
	dxrCommandList->SetPipelineState1(stateObject.Get());

	gfx.DispatchRays(GetDispatchRaysDesc());
}
}
}