#include "UploadBuffer.h"
#include "engine/Debug/Logger/Log.h"

#include "../GraphicsCore.h"

namespace NoEngine {
void UploadBuffer::Create(const std::wstring& name, size_t BufferSize) {
    Log::DebugPrint("UploadBuffer Create Start", VerbosityLevel::kDebug);
    Destroy();

    bufferSize_ = BufferSize;

    // アップロードバッファを作成します。これはCPUから参照できますが、書き込み結合メモリなので、
    // そこからの読み出しは避けてください。
    D3D12_HEAP_PROPERTIES heapProps;
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    // アップロードバッファは1次元である必要があります
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

    HRESULT hr = GraphicsCore::gGraphicsDevice->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource_));

    assert(SUCCEEDED(hr));

    gpuVirtualAddress_ = resource_->GetGPUVirtualAddress();

#ifdef RELEASE_BUILD
    static_cast<void>(name);
#else
    resource_->SetName(name.c_str());
#endif
    Log::DebugPrint("UploadBuffer Created", VerbosityLevel::kDebug);
}

void* UploadBuffer::Map(void) {

    void* Memory;
    CD3DX12_RANGE cd3dx12Range(0, bufferSize_);
    resource_->Map(0, &cd3dx12Range, &Memory);
    return Memory;
}

void UploadBuffer::Unmap(size_t begin, size_t end) {
    CD3DX12_RANGE cd3dx12Range(begin, std::min(end, bufferSize_));
    resource_->Unmap(0, &cd3dx12Range);
}

}
