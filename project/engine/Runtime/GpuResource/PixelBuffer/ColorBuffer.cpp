#include "ColorBuffer.h"

#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine {
void ColorBuffer::CreateFromSwapChain(const std::wstring& name, ID3D12Resource* baseResource) {
	AssociateWithResource(name, baseResource, D3D12_RESOURCE_STATE_PRESENT);
	rtvHandle_ = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_RENDER_TARGET_VIEW_DESC desc{};
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	GraphicsCore::sGraphicsDevice->GetDevice()->CreateRenderTargetView(resource_.Get(), &desc, rtvHandle_);
}

void ColorBuffer::Create(const std::wstring& name, uint32_t width, uint32_t height, uint32_t numMips, DXGI_FORMAT format) {
    Create(name, width, height, 1, numMips, format);
}

void ColorBuffer::Create(const std::wstring& name, uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t numMips, DXGI_FORMAT format) {
    numMips = (numMips == 0 ? ComputeNumMips(width, height) : numMips);
    D3D12_RESOURCE_FLAGS Flags = CombineResourceFlags();
    D3D12_RESOURCE_DESC ResourceDesc = DescribeTex2D(width, height, depthOrArraySize, numMips, format, Flags);

    ResourceDesc.SampleDesc.Count = fragmentCount_;
    ResourceDesc.SampleDesc.Quality = 0;

    D3D12_CLEAR_VALUE ClearValue = {};
    ClearValue.Format = format;
    ClearValue.Color[0] = clearColor_.r;
    ClearValue.Color[1] = clearColor_.g;
    ClearValue.Color[2] = clearColor_.b;
    ClearValue.Color[3] = clearColor_.a;

    CreateTextureResource(GraphicsCore::sGraphicsDevice->GetDevice(), name, ResourceDesc, ClearValue);
    CreateDerivedViews(GraphicsCore::sGraphicsDevice->GetDevice(), format, depthOrArraySize, numMips);
}

void ColorBuffer::CreateDerivedViews(ID3D12Device* Device, DXGI_FORMAT Format, uint32_t ArraySize, uint32_t NumMips) {
    if (ArraySize != 1 && NumMips != 1) {
        LogCritical("We don't support auto-mips on mTexture arrays");
        assert(false);
    }

    numMipMaps_ = NumMips - 1;

    D3D12_RENDER_TARGET_VIEW_DESC RTVDesc = {};
    D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
    D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};

    RTVDesc.Format = Format;
    UAVDesc.Format = GetUAVFormat(Format);
    SRVDesc.Format = Format;
    SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    if (ArraySize > 1) {
        RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
        RTVDesc.Texture2DArray.MipSlice = 0;
        RTVDesc.Texture2DArray.FirstArraySlice = 0;
        RTVDesc.Texture2DArray.ArraySize = (UINT)ArraySize;

        UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        UAVDesc.Texture2DArray.MipSlice = 0;
        UAVDesc.Texture2DArray.FirstArraySlice = 0;
        UAVDesc.Texture2DArray.ArraySize = (UINT)ArraySize;

        SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        SRVDesc.Texture2DArray.MipLevels = NumMips;
        SRVDesc.Texture2DArray.MostDetailedMip = 0;
        SRVDesc.Texture2DArray.FirstArraySlice = 0;
        SRVDesc.Texture2DArray.ArraySize = (UINT)ArraySize;
    } else if (fragmentCount_ > 1) {
        RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
        SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
    } else {
        RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        RTVDesc.Texture2D.MipSlice = 0;

        UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        UAVDesc.Texture2D.MipSlice = 0;

        SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        SRVDesc.Texture2D.MipLevels = NumMips;
        SRVDesc.Texture2D.MostDetailedMip = 0;
    }

    if (srvHandle_.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN) {
        rtvHandle_ = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        srvHandle_ = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    ID3D12Resource* Resource = resource_.Get();

    // rtvを作成
    Device->CreateRenderTargetView(Resource, &RTVDesc, rtvHandle_);

    // srvを作成
    Device->CreateShaderResourceView(Resource, &SRVDesc, srvHandle_);

    if (fragmentCount_ > 1)
        return;

    // Create the UAVs for each mip level (RWTexture2D)
    for (uint32_t i = 0; i < NumMips; ++i) {
        if (uavHandle_[i].ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
            uavHandle_[i] = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        Device->CreateUnorderedAccessView(Resource, nullptr, &UAVDesc, uavHandle_[i]);

        UAVDesc.Texture2D.MipSlice++;
    }
}
}