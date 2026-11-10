#pragma once
#include "engine/Runtime/PipelineStateObject/ComputePSO.h"
#include "engine/Runtime/PipelineStateObject/GraphicsPSO.h"
#include "engine/Runtime/PipelineStateObject/RootSignature.h"
#include "engine/Runtime/DescriptorHeap/DescriptorHeap.h"

#include <dxcapi.h>

namespace NoEngine {
namespace Render {
void Initialize();
void Shutdown();
extern DescriptorHeap gTextureHeap;

RootSignature& GetRootSignature(uint32_t rootSigId);
uint32_t GetRootSignatureID(std::wstring rootSigName);

Microsoft::WRL::ComPtr<ID3D12StateObject>& GetShadowRtStateObject();
D3D12_DISPATCH_RAYS_DESC& GetShadowDispatchRaysDesc();
const ComputePSO& GetShadowFilterPSO();

ID3D12StateObject* GetFogRtStateObject();
const RootSignature& GetFogRtRootSignature();
const D3D12_DISPATCH_RAYS_DESC& GetFogDispatchRaysDesc();

Microsoft::WRL::ComPtr<ID3D12StateObject>& GetRtStateObject();
D3D12_DISPATCH_RAYS_DESC& GetDispatchRaysDesc();

void ResizeRaytracingDesc(UINT width, UINT height);

void InitRaytracingGlobalRootSignature();
void InitRaytracingLocalRootSignature();
}
}
