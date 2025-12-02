#include "ColorBuffer.h"

#include "engine/GameCore/GraphicsCore.h"

namespace NoEngine {
void ColorBuffer::CreateFromSwapChain(const std::wstring& name, ID3D12Resource* baseResource) {
	AssociateWithResource(name, baseResource, D3D12_RESOURCE_STATE_PRESENT);
	rtvHandle_ = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_RENDER_TARGET_VIEW_DESC desc{};
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	GraphicsCore::gGraphicsDevice->GetDevice()->CreateRenderTargetView(resource_.Get(), &desc, rtvHandle_);
}
}