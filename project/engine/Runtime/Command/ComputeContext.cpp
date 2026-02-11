#include "ComputeContext.h"
#include "../GraphicsCore.h"
#include "engine/Math/Common.h"

namespace NoEngine {

ComputeContext& ComputeContext::Begin(const std::wstring& id, bool async) {
    ComputeContext& NewContext = GraphicsCore::gContextManager.AllocateContext(
        async ? D3D12_COMMAND_LIST_TYPE_COMPUTE : D3D12_COMMAND_LIST_TYPE_DIRECT)->GetComputeContext();
    NewContext.SetID(id);
    return NewContext;
}

void ComputeContext::ClearUAV(GpuBuffer& Target) {
    FlushResourceBarriers();

    // UAVをバインドした後、UAVとしてクリアするために必要なGPUハンドルを取得できます(基本的にすべての値を設定するシェーダーを実行するため)。
    D3D12_GPU_DESCRIPTOR_HANDLE GpuVisibleHandle = dynamicViewDescriptorHeap_.UploadDirect(Target.GetUAV());
    const UINT ClearColor[4] = {};
    commandList_->ClearUnorderedAccessViewUint(GpuVisibleHandle, Target.GetUAV(), Target.GetResource(), ClearColor, 0, nullptr);
}

void ComputeContext::ClearUAV(ColorBuffer& Target) {
    FlushResourceBarriers();

    D3D12_GPU_DESCRIPTOR_HANDLE GpuVisibleHandle = dynamicViewDescriptorHeap_.UploadDirect(Target.GetUAV());
    CD3DX12_RECT ClearRect(0, 0, (LONG)Target.GetWidth(), (LONG)Target.GetHeight());

    const float* ClearColor = Target.GetClearColor().ptr();
    commandList_->ClearUnorderedAccessViewFloat(GpuVisibleHandle, Target.GetUAV(), Target.GetResource(), ClearColor, 1, &ClearRect);
}

void ComputeContext::SetRootSignature(const RootSignature& RootSig) {
    if (RootSig.GetSignature() == curComputeRootSignature_) return;

    commandList_->SetComputeRootSignature(curComputeRootSignature_ = RootSig.GetSignature());

    dynamicViewDescriptorHeap_.ParseComputeRootSignature(RootSig);
    dynamicSamplerDescriptorHeap_.ParseComputeRootSignature(RootSig);
}


void ComputeContext::SetConstantBuffer(UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS cbv) {
    commandList_->SetComputeRootConstantBufferView(rootIndex, cbv);
}

void ComputeContext::SetDynamicConstantBufferView(UINT RootIndex, size_t BufferSize, const void* BufferData) {
    assert(BufferData != nullptr && Math::IsAligned(BufferData, 16));
    DynAlloc cb = cpuLinearAllocator_.Allocate(BufferSize);
    memcpy(cb.DataPtr, BufferData, BufferSize);
    commandList_->SetComputeRootConstantBufferView(RootIndex, cb.GpuAddress);
}

void ComputeContext::SetBufferSRV(UINT RootIndex, const GpuBuffer& SRV, UINT64 Offset) {
    assert((SRV.usageState_ & (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)) != 0);
    commandList_->SetComputeRootShaderResourceView(RootIndex, SRV.GetGpuVirtualAddress() + Offset);
}

void ComputeContext::SetBufferUAV(UINT RootIndex, const GpuBuffer& UAV, UINT64 Offset) {
    assert((UAV.usageState_ & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) != 0);
    commandList_->SetComputeRootUnorderedAccessView(RootIndex, UAV.GetGpuVirtualAddress() + Offset);
}

void ComputeContext::SetDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE FirstHandle) {
    commandList_->SetComputeRootDescriptorTable(RootIndex, FirstHandle);
}

void ComputeContext::SetDynamicDescriptor(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle) {
    SetDynamicDescriptors(RootIndex, Offset, 1, &Handle);
}

void ComputeContext::SetDynamicDescriptors(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]) {
    dynamicViewDescriptorHeap_.SetComputeDescriptorHandles(RootIndex, Offset, Count, Handles);
}

void ComputeContext::Dispatch(size_t GroupCountX, size_t GroupCountY, size_t GroupCountZ) {
    FlushResourceBarriers();
    dynamicViewDescriptorHeap_.CommitComputeRootDescriptorTables(commandList_);
    dynamicSamplerDescriptorHeap_.CommitComputeRootDescriptorTables(commandList_);
    commandList_->Dispatch((UINT)GroupCountX, (UINT)GroupCountY, (UINT)GroupCountZ);
}

void ComputeContext::Dispatch1D(size_t ThreadCountX, size_t GroupSizeX) {
    Dispatch(Math::DivideByMultiple(ThreadCountX, GroupSizeX), 1, 1);
}

void ComputeContext::Dispatch2D(size_t ThreadCountX, size_t ThreadCountY, size_t GroupSizeX, size_t GroupSizeY) {
    Dispatch(
        Math::DivideByMultiple(ThreadCountX, GroupSizeX),
        Math::DivideByMultiple(ThreadCountY, GroupSizeY), 1);
}

void ComputeContext::Dispatch3D(size_t ThreadCountX, size_t ThreadCountY, size_t ThreadCountZ, size_t GroupSizeX, size_t GroupSizeY, size_t GroupSizeZ) {
    Dispatch(
        Math::DivideByMultiple(ThreadCountX, GroupSizeX),
        Math::DivideByMultiple(ThreadCountY, GroupSizeY),
        Math::DivideByMultiple(ThreadCountZ, GroupSizeZ));
}
}