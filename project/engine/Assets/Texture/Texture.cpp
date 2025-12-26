#include "Texture.h"
#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine {
void Texture::Create2D(size_t rowPitchBytes, size_t width, size_t height, DXGI_FORMAT format, const void* initData) {
    Destroy();

    usageState_ = D3D12_RESOURCE_STATE_COPY_DEST;

    width_ = (uint32_t)width;
    height_ = (uint32_t)height;
    depth_ = 1;

    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Width = width;
    texDesc.Height = (UINT)height;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = format;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    D3D12_HEAP_PROPERTIES HeapProps;
    HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    HeapProps.CreationNodeMask = 1;
    HeapProps.VisibleNodeMask = 1;

    HRESULT hr = GraphicsCore::gGraphicsDevice->GetDevice()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
        usageState_, nullptr, IID_PPV_ARGS(resource_.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) {
        assert(false);
    }

    resource_->SetName(L"Texture");

    D3D12_SUBRESOURCE_DATA texResource;
    texResource.pData = initData;
    texResource.RowPitch = rowPitchBytes;
    texResource.SlicePitch = rowPitchBytes * height;

    CommandContext::InitializeTexture(*this, 1, &texResource);

    if (cpuDescriptorHandle_.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
        cpuDescriptorHandle_ = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    GraphicsCore::gGraphicsDevice->GetDevice()->CreateShaderResourceView(resource_.Get(), nullptr, cpuDescriptorHandle_);
}
}