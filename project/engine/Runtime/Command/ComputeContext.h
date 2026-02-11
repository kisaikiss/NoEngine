#pragma once
#include "CommandContext.h"
namespace NoEngine {

struct DWParam {
    DWParam(FLOAT f) : Float(f) {}
    DWParam(UINT u) : Uint(u) {}
    DWParam(INT i) : Int(i) {}

    void operator= (FLOAT f) { Float = f; }
    void operator= (UINT u) { Uint = u; }
    void operator= (INT i) { Int = i; }

    union {
        FLOAT Float;
        UINT Uint;
        INT Int;
    };
};

class ComputeContext : public CommandContext {
public:
	static ComputeContext& Begin(const std::wstring& id = L"", bool async = false);

    void ClearUAV(GpuBuffer& Target);
    void ClearUAV(ColorBuffer& Target);

    void SetRootSignature(const RootSignature& RootSig);

    void SetConstantBuffer(UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS CBV);
    void SetDynamicConstantBufferView(UINT RootIndex, size_t BufferSize, const void* BufferData);
    void SetDynamicSRV(UINT RootIndex, size_t BufferSize, const void* BufferData);
    void SetBufferSRV(UINT RootIndex, const GpuBuffer& SRV, UINT64 Offset = 0);
    void SetBufferUAV(UINT RootIndex, const GpuBuffer& UAV, UINT64 Offset = 0);
    void SetDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE FirstHandle);

    void SetDynamicDescriptor(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle);
    void SetDynamicDescriptors(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]);
  
    void Dispatch(size_t GroupCountX = 1, size_t GroupCountY = 1, size_t GroupCountZ = 1);
    void Dispatch1D(size_t ThreadCountX, size_t GroupSizeX);
    void Dispatch2D(size_t ThreadCountX, size_t ThreadCountY, size_t GroupSizeX = 8, size_t GroupSizeY = 8);
    void Dispatch3D(size_t ThreadCountX, size_t ThreadCountY, size_t ThreadCountZ, size_t GroupSizeX, size_t GroupSizeY, size_t GroupSizeZ);
};
}

