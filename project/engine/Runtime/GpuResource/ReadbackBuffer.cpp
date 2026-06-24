#include "stdafx.h"
#include "ReadbackBuffer.h"
#include "../GraphicsCore.h"

namespace NoEngine {
void ReadbackBuffer::Create(const std::wstring& name, uint32_t numElements, uint32_t elementSize) {
    LogDebug("ReadbackBuffer Create Start");
    Destroy();

    elementCount_ = numElements;
    elementSize_ = elementSize;
    bufferSize_ = numElements * elementSize;
    usageState_ = D3D12_RESOURCE_STATE_COPY_DEST;

    // すべてのテクセルデータを保持するのに十分な大きさのリードバックバッファを作成する
    D3D12_HEAP_PROPERTIES heapProps;
    heapProps.Type = D3D12_HEAP_TYPE_READBACK;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    // リードバックバッファは1次元である必要があります。「texture2d」ではなく「buffer」です。
    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = bufferSize_;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    HRESULT hr = GraphicsCore::sGraphicsDevice->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&resource_));

    if (FAILED(hr)) {
        LogCritical("ReadbackBuffer Create failed");
        assert(false);
    }

    gpuVirtualAddress_ = resource_->GetGPUVirtualAddress();

#ifdef _DEBUG
    resource_->SetName(name.c_str());
#else
    static_cast<void>(name);
#endif

    LogDebug("ReadbackBuffer Created");
}

void* ReadbackBuffer::Map(void) {
    void* memory;
    CD3DX12_RANGE range(0, bufferSize_);
    resource_->Map(0, &range, &memory);
    return memory;
}

void ReadbackBuffer::Unmap(void) {
    CD3DX12_RANGE range(0, 0);
    resource_->Unmap(0, &range);
}

}