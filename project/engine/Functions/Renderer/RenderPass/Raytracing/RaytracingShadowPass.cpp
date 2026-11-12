#include "RaytracingShadowPass.h"

#include "../../RaytracingManager.h"
#include "engine/Functions/ECS/Component/Common/CameraComponent.h"
#include "engine/Runtime/Command/ComputeContext.h"

#include <bit>

namespace NoEngine {
namespace Render {

namespace {
void FilterSoftShadowSlices(GraphicsContext& gfx, ColorBuffer& shadowMask, ColorBuffer& scratch,
	const ColorBuffer& worldPosition, const ColorBuffer& normal, const Math::Matrix4x4& view, uint64_t slices) {
	auto& compute = gfx.GetComputeContext();
	const auto& pipeline = GetShadowFilterPSO();
	compute.SetRootSignature(pipeline.GetRootSignature());
	compute.SetPipelineState(pipeline);

	const D3D12_CPU_DESCRIPTOR_HANDLE inputs[] = { shadowMask.GetSRV(), worldPosition.GetSRV(), normal.GetSRV() };
	compute.SetDynamicDescriptors(0, 0, _countof(inputs), inputs);
	compute.SetDynamicDescriptor(1, 0, scratch.GetUAV());

	struct alignas(16) FilterConstants {
		float viewDepthRow[4];
		uint32_t shadowSlice;
		uint32_t padding[3]{};
	} constants{ { view.m[0][2], view.m[1][2], view.m[2][2], view.m[3][2] }, 0 };

	while (slices != 0) {
		constants.shadowSlice = static_cast<uint32_t>(std::countr_zero(slices));
		slices &= slices - 1;

		compute.TransitionResource(shadowMask, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		compute.TransitionResource(scratch, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		compute.SetDynamicConstantBufferView(2, sizeof(constants), &constants);
		compute.Dispatch2D(scratch.GetWidth(), scratch.GetHeight());

		// フィルタ結果を対象のライトのスライスだけに戻す。
		compute.TransitionResource(scratch, D3D12_RESOURCE_STATE_COPY_SOURCE);
		compute.TransitionResource(shadowMask, D3D12_RESOURCE_STATE_COPY_DEST);
		compute.FlushResourceBarriers();
		const CD3DX12_TEXTURE_COPY_LOCATION destination(shadowMask.GetResource(), constants.shadowSlice);
		const CD3DX12_TEXTURE_COPY_LOCATION source(scratch.GetResource(), 0);
		compute.GetCommandList()->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);
	}
}
}

void RaytracingShadowPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	static_cast<void>(registry);
	Dispatch(gfx, resourceRegistry);
}

void RaytracingShadowPass::Dispatch(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry) {
	auto& stateObject = Render::GetShadowRtStateObject();
	auto* renderContext = GetRenderContext();
	auto* camera = GetTargetCamera();

	auto* lightNums = renderContext->GetLightNums();
	if (!renderContext->GetTLAS().Get() || !camera) return;
	if (lightNums->directionalLightNum == 0 &&
		lightNums->pointLightNum == 0 &&
		lightNums->spotLightNum == 0) return;

	auto& worldPosition = const_cast<ColorBuffer&>(resourceRegistry.GetColorBuffer(GetInput("WorldPosition")));
	auto& normal = const_cast<ColorBuffer&>(resourceRegistry.GetColorBuffer(GetInput("Normal")));
	auto& shadowMask = const_cast<ColorBuffer&>(resourceRegistry.GetColorBuffer(GetEditorUavOutputs().at(0)));
	gfx.TransitionResource(worldPosition, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	gfx.TransitionResource(normal, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	gfx.SetRaytracingRootSignature(Render::GetRootSignature(Render::GetRootSignatureID(L"RT Global RootSignature")));

	gfx.SetComputeSRV(0, renderContext->GetTLAS()->GetGPUVirtualAddress());
	gfx.SetRaytracingDynamicDescriptor(1, 0, worldPosition.GetSRV());
	gfx.SetRaytracingDynamicDescriptor(5, 0, normal.GetSRV());

	gfx.SetRaytracingDynamicDescriptor(2, 0, renderContext->GetDirectionalLightSRV());
	gfx.SetRaytracingDynamicDescriptor(6, 0, renderContext->GetPointLightSRV());
	gfx.SetRaytracingDynamicDescriptor(7, 0, renderContext->GetSpotLightSRV());

	gfx.SetRaytracingDynamicDescriptor(3, 0, shadowMask.GetUAV());


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
	gfx.SetRaytracingDynamicConstantBufferView(4, sizeof(constants), &constants);

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> dxrCommandList;
	gfx.GetCommandList()->QueryInterface(IID_PPV_ARGS(&dxrCommandList));
	dxrCommandList->SetPipelineState1(stateObject.Get());

	gfx.DispatchRays(GetShadowDispatchRaysDesc());
	const uint64_t softShadowSlices = renderContext->GetSoftShadowSlices();
	if (softShadowSlices != 0) {
		auto& scratch = const_cast<ColorBuffer&>(resourceRegistry.GetColorBuffer("ShadowFilterScratch"));
		FilterSoftShadowSlices(gfx, shadowMask, scratch, worldPosition, normal, camera->view, softShadowSlices);
	}
}
}
}

