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
    D3D12_HEAP_PROPERTIES HeapProps;
    HeapProps.Type = D3D12_HEAP_TYPE_READBACK;
    HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    HeapProps.CreationNodeMask = 1;
    HeapProps.VisibleNodeMask = 1;

    // リードバックバッファは1次元である必要があります。「texture2d」ではなく「buffer」です。
    D3D12_RESOURCE_DESC ResourceDesc = {};
    ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    ResourceDesc.Width = bufferSize_;
    ResourceDesc.Height = 1;
    ResourceDesc.DepthOrArraySize = 1;
    ResourceDesc.MipLevels = 1;
    ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    ResourceDesc.SampleDesc.Count = 1;
    ResourceDesc.SampleDesc.Quality = 0;
    ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    HRESULT hr = GraphicsCore::sGraphicsDevice->GetDevice()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &ResourceDesc,
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
    void* Memory;
    CD3DX12_RANGE range(0, bufferSize_);
    resource_->Map(0, &range, &Memory);
    return Memory;
}

void ReadbackBuffer::Unmap(void) {
    CD3DX12_RANGE range(0, 0);
    resource_->Unmap(0, &range);
}

}