#pragma once
#include "engine/Runtime/PipelineStateObject/GraphicsPSO.h"
#include "engine/Runtime/PipelineStateObject/RootSignature.h"
#include "engine/Runtime/DescriptorHeap/DescriptorHeap.h"

#include <dxcapi.h>

namespace NoEngine {
namespace Render {
void Initialize();
void Shutdown();
extern DescriptorHeap gTextureHeap;

GraphicsPSO& GetPSO(uint32_t psoId);
RootSignature& GetRootSignature(uint32_t rootSigId);
uint32_t GetPSOID(std::wstring psoName);
uint32_t GetRootSignatureID(std::wstring rootSigName);

Microsoft::WRL::ComPtr<ID3D12StateObject>& GetShadowRtStateObject();
D3D12_DISPATCH_RAYS_DESC& GetShadowDispatchRaysDesc();

Microsoft::WRL::ComPtr<ID3D12StateObject>& GetRtStateObject();
D3D12_DISPATCH_RAYS_DESC& GetDispatchRaysDesc();

void CreateShadowShaderTable();

void InitRaytracingGlobalRootSignature();
void InitRaytracingLocalRootSignature();
}
}