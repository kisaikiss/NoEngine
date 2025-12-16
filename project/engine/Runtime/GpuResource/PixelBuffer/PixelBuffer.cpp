#include "PixelBuffer.h"
namespace NoEngine {
D3D12_RESOURCE_DESC PixelBuffer::DescribeTex2D(uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t numMips, DXGI_FORMAT format, UINT flags) {
	width_ = width;
	height_ = height;
	arraySize_ = depthOrArraySize;
	format_ = format;

	D3D12_RESOURCE_DESC desc{};

    desc.Alignment = 0;
    desc.DepthOrArraySize = static_cast<UINT16>(depthOrArraySize);
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Flags = static_cast<D3D12_RESOURCE_FLAGS>(flags);
    desc.Format = GetBaseFormat(format);
    desc.Height = static_cast<UINT>(height);
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.MipLevels = static_cast<UINT16>(numMips);
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Width = static_cast<UINT16>(width);
    return desc;


}

void PixelBuffer::AssociateWithResource(const std::wstring& name, ID3D12Resource* resource, D3D12_RESOURCE_STATES currentState) {
    assert(resource != nullptr);
    D3D12_RESOURCE_DESC resourceDesc = resource->GetDesc();

    resource_.Attach(resource);
    usageState_ = currentState;

    width_ = static_cast<uint32_t>(resourceDesc.Width);
    height_ = static_cast<uint32_t>(resourceDesc.Height);
    arraySize_ = static_cast<uint32_t>(resourceDesc.DepthOrArraySize);
    format_ = resourceDesc.Format;

#ifndef RELEASE_BUILD
    resource->SetName(name.c_str());
#else
    static_cast<void>(name);
#endif
}

DXGI_FORMAT PixelBuffer::GetBaseFormat(DXGI_FORMAT format) {

    switch (format) {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        return DXGI_FORMAT_R8G8B8A8_TYPELESS;

    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        return DXGI_FORMAT_B8G8R8A8_TYPELESS;

    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        return DXGI_FORMAT_B8G8R8X8_TYPELESS;

        // 32-bit Z w/ Stencil
    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        return DXGI_FORMAT_R32G8X24_TYPELESS;

        // No Stencil
    case DXGI_FORMAT_R32_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
        return DXGI_FORMAT_R32_TYPELESS;

        // 24-bit Z
    case DXGI_FORMAT_R24G8_TYPELESS:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        return DXGI_FORMAT_R24G8_TYPELESS;

        // 16-bit Z w/o Stencil
    case DXGI_FORMAT_R16_TYPELESS:
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UNORM:
        return DXGI_FORMAT_R16_TYPELESS;

    default:
        return format;
    }
}

}