#include "ColorBuffer.h"

#include "engine/GameCore/GraphicsCore.h"

namespace NoEngine {
void ColorBuffer::CreateFromSwapChain(const std::wstring& name, ID3D12Resource* baseResource) {
	AssociateWithResource(name, baseResource, D3D12_RESOURCE_STATE_PRESENT);
	rtvHandle_ = GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	GraphicsCore::gGraphicsDevice->GetDevice()->CreateRenderTargetView(resource_.Get(), nullptr, rtvHandle_);
}
}