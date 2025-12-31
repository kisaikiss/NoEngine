#include "DepthBuffer.h"
#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine {
void DepthBuffer::Create(const std::wstring& name, uint32_t width, uint32_t height, DXGI_FORMAT format) {
	Create(name, width, height, 1, format);
}

void DepthBuffer::Create(const std::wstring& name, uint32_t width, uint32_t height, uint32_t numSamples, DXGI_FORMAT format) {
    D3D12_RESOURCE_DESC ResourceDesc = DescribeTex2D(width, height, 1, 1, format, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
    ResourceDesc.SampleDesc.Count = numSamples;

    D3D12_CLEAR_VALUE ClearValue = {};
    ClearValue.Format = format;
    ClearValue.DepthStencil.Depth = clearDepth_;
    ClearValue.DepthStencil.Stencil = clearStencil_;
    CreateTextureResource(GraphicsCore::gGraphicsDevice->GetDevice(), name, ResourceDesc, ClearValue);
    CreateDerivedViews(GraphicsCore::gGraphicsDevice->GetDevice(), format);
}

void DepthBuffer::CreateDerivedViews(ID3D12Device* device, DXGI_FORMAT format) {
    ID3D12Resource* Resource = resource_.Get();

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Format = GetDSVFormat(format);
    if (Resource->GetDesc().SampleDesc.Count == 1) {
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;
    } else {
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
    }

    if (dsvHandle_[0].ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN) {
        dsvHandle_[0] = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        dsvHandle_[1] = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    }

    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    device->CreateDepthStencilView(Resource, &dsvDesc, dsvHandle_[0]);

    dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
    device->CreateDepthStencilView(Resource, &dsvDesc, dsvHandle_[1]);

    DXGI_FORMAT stencilReadFormat = GetStencilFormat(format);
    if (stencilReadFormat != DXGI_FORMAT_UNKNOWN) {
        if (dsvHandle_[2].ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN) {
            dsvHandle_[2] = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
            dsvHandle_[3] = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        }

        dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_STENCIL;
        device->CreateDepthStencilView(Resource, &dsvDesc, dsvHandle_[2]);

        dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH | D3D12_DSV_FLAG_READ_ONLY_STENCIL;
        device->CreateDepthStencilView(Resource, &dsvDesc, dsvHandle_[3]);
    } else {
        dsvHandle_[2] = dsvHandle_[0];
        dsvHandle_[3] = dsvHandle_[1];
    }

    if (depthSRVHandle_.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
        depthSRVHandle_ = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // SRVを作成します。
    D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.Format = GetDepthFormat(format);
    if (dsvDesc.ViewDimension == D3D12_DSV_DIMENSION_TEXTURE2D) {
        SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        SRVDesc.Texture2D.MipLevels = 1;
    } else {
        SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
    }
    SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    device->CreateShaderResourceView(Resource, &SRVDesc, depthSRVHandle_);

    if (stencilReadFormat != DXGI_FORMAT_UNKNOWN) {
        if (stencilSRVHandle_.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
            stencilSRVHandle_ = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        SRVDesc.Format = stencilReadFormat;
        SRVDesc.Texture2D.PlaneSlice = 1;

        device->CreateShaderResourceView(Resource, &SRVDesc, stencilSRVHandle_);
    }
}

}