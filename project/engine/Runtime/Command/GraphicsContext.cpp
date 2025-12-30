#include "GraphicsContext.h"

#include "../GpuResource/PixelBuffer/ColorBuffer.h"
#include "engine/Math/Common.h"

namespace NoEngine {
void GraphicsContext::ClearColor(ColorBuffer& target) {
	FlushResourceBarriers();
	const Color& colorVec = target.GetClearColor();
	float color[] = { colorVec.r, colorVec.g, colorVec.b, colorVec.a };
	commandList_->ClearRenderTargetView(target.GetRTV(), color, 0, nullptr);
}
void GraphicsContext::ClearColor(ColorBuffer& target, float color[4]) {
	FlushResourceBarriers();
	commandList_->ClearRenderTargetView(target.GetRTV(), color, 0, nullptr);
}

void GraphicsContext::SetRootSignature(const RootSignature& rootSig) {
	if (rootSig.GetSignature() == curGraphicsRootSignature_)
		return;

	commandList_->SetGraphicsRootSignature(curGraphicsRootSignature_ = rootSig.GetSignature());

	dynamicViewDescriptorHeap_.ParseGraphicsRootSignature(rootSig);
	dynamicSamplerDescriptorHeap_.ParseGraphicsRootSignature(rootSig);
}

void GraphicsContext::SetRenderTargets(UINT NumRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[]) {
	commandList_->OMSetRenderTargets(NumRTVs, RTVs, FALSE, nullptr);
}
void GraphicsContext::SetRenderTargets(UINT NumRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[], D3D12_CPU_DESCRIPTOR_HANDLE DSV) {
	commandList_->OMSetRenderTargets(NumRTVs, RTVs, FALSE, &DSV);
}

void GraphicsContext::SetViewport(const D3D12_VIEWPORT& vp) {
	commandList_->RSSetViewports(1, &vp);
}
void GraphicsContext::SetViewport(FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT minDepth, FLOAT maxDepth) {
	D3D12_VIEWPORT vp;
	vp.Width = w;
	vp.Height = h;
	vp.MinDepth = minDepth;
	vp.MaxDepth = maxDepth;
	vp.TopLeftX = x;
	vp.TopLeftY = y;
	commandList_->RSSetViewports(1, &vp);
}

void GraphicsContext::SetScissor(const D3D12_RECT& rect) {
	commandList_->RSSetScissorRects(1, &rect);
}
void GraphicsContext::SetScissor(UINT left, UINT top, UINT right, UINT bottom) {
	SetScissor(CD3DX12_RECT(left, top, right, bottom));
}

void GraphicsContext::SetViewportAndScissor(const D3D12_VIEWPORT& vp, const D3D12_RECT& rect) {
	commandList_->RSSetViewports(1, &vp);
	commandList_->RSSetScissorRects(1, &rect);
}
void GraphicsContext::SetViewportAndScissor(UINT x, UINT y, UINT w, UINT h) {
	SetViewport((float)x, (float)y, (float)w, (float)h);
	SetScissor(x, y, x + w, y + h);
}

void GraphicsContext::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology) {
	commandList_->IASetPrimitiveTopology(Topology);
}

void GraphicsContext::SetConstantBuffer(UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS cbv) {
	commandList_->SetGraphicsRootConstantBufferView(rootIndex, cbv);
}

void GraphicsContext::SetDynamicConstantBufferView(UINT RootIndex, size_t BufferSize, const void* BufferData) {
	assert(BufferData != nullptr && Math::IsAligned(BufferData, 16));
	DynAlloc cb = cpuLinearAllocator_.Allocate(BufferSize);
	//SIMDMemCopy(cb.DataPtr, BufferData, Math::AlignUp(BufferSize, 16) >> 4);
	memcpy(cb.DataPtr, BufferData, BufferSize);
	commandList_->SetGraphicsRootConstantBufferView(RootIndex, cb.GpuAddress);
}

void GraphicsContext::SetBufferSRV(UINT RootIndex, const GpuBuffer& SRV, UINT64 Offset) {
	assert((SRV.usageState_ & (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)) != 0);
	commandList_->SetGraphicsRootShaderResourceView(RootIndex, SRV.GetGpuVirtualAddress() + Offset);
}

void GraphicsContext::SetDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE FirstHandle) {
	commandList_->SetGraphicsRootDescriptorTable(RootIndex, FirstHandle);
}

void GraphicsContext::SetDynamicDescriptor(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle) {
	SetDynamicDescriptors(RootIndex, Offset, 1, &Handle);
}

void GraphicsContext::SetDynamicDescriptors(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]) {
	dynamicViewDescriptorHeap_.SetGraphicsDescriptorHandles(RootIndex, Offset, Count, Handles);
}

void GraphicsContext::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& ibview) {
	commandList_->IASetIndexBuffer(&ibview);
}
void GraphicsContext::SetVertexBuffer(UINT slot, const D3D12_VERTEX_BUFFER_VIEW& vbview) {
	SetVertexBuffers(slot, 1, &vbview);
}
void GraphicsContext::SetVertexBuffers(UINT startSlot, UINT count, const D3D12_VERTEX_BUFFER_VIEW vbviews[]) {
	commandList_->IASetVertexBuffers(startSlot, count, vbviews);
}

void GraphicsContext::SetDynamicSRV(UINT RootIndex, size_t BufferSize, const void* BufferData) {
	assert(BufferData != nullptr && Math::IsAligned(BufferData, 16));
	DynAlloc cb = cpuLinearAllocator_.Allocate(BufferSize);
	std::memcpy(cb.DataPtr, BufferData, Math::AlignUp(BufferSize, 16) >> 4);
	commandList_->SetGraphicsRootShaderResourceView(RootIndex, cb.GpuAddress);
}

void GraphicsContext::Draw(UINT vertexCount, UINT vertexStartOffset) {
	DrawInstanced(vertexCount, 1, vertexStartOffset, 0);
}

void GraphicsContext::DrawInstanced(UINT vertexCountPerInstance, UINT instanceCount, UINT StartVertexLocation, UINT startInstanceLocation) {
	FlushResourceBarriers();
	dynamicViewDescriptorHeap_.CommitGraphicsRootDescriptorTables(commandList_);
	dynamicSamplerDescriptorHeap_.CommitGraphicsRootDescriptorTables(commandList_);
	commandList_->DrawInstanced(vertexCountPerInstance, instanceCount, StartVertexLocation, startInstanceLocation);
}
}
