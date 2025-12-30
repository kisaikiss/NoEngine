#pragma once
#include "CommandContext.h"

namespace NoEngine {

class GraphicsContext : public CommandContext {
public:
    static GraphicsContext& Begin(const std::wstring& ID = L"") {
        return CommandContext::Begin(ID).GetGraphicsContext();
    }

    void ClearColor(ColorBuffer& target);
    void ClearColor(ColorBuffer& target, float color[4]);

    void SetRootSignature(const RootSignature& rootSig);

    void SetRenderTargets(UINT NumRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[]);
    void SetRenderTargets(UINT NumRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[], D3D12_CPU_DESCRIPTOR_HANDLE DSV);
    void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE RTV) { SetRenderTargets(1, &RTV); }

    void SetViewport(const D3D12_VIEWPORT& vp);
    void SetViewport(FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT minDepth = 0.0f, FLOAT maxDepth = 1.0f);
    void SetScissor(const D3D12_RECT& rect);
    void SetScissor(UINT left, UINT top, UINT right, UINT bottom);
    void SetViewportAndScissor(const D3D12_VIEWPORT& vp, const D3D12_RECT& rect);
    void SetViewportAndScissor(UINT x, UINT y, UINT w, UINT h);
    void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology);

    void SetConstantBuffer(UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS cbv);
    void SetDynamicConstantBufferView(UINT RootIndex, size_t BufferSize, const void* BufferData);
    void SetBufferSRV(UINT RootIndex, const GpuBuffer& SRV, UINT64 Offset = 0);
    void SetDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE FirstHandle);

    void SetDynamicDescriptor(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle);
    void SetDynamicDescriptors(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]);

    void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& ibview);
    void SetVertexBuffer(UINT slot, const D3D12_VERTEX_BUFFER_VIEW& vbview);
    void SetVertexBuffers(UINT startSlot, UINT count, const D3D12_VERTEX_BUFFER_VIEW vbviews[]);
    void SetDynamicSRV(UINT RootIndex, size_t BufferSize, const void* BufferData);

    void Draw(UINT vertexCount, UINT vertexStartOffset = 0);
    void DrawIndexed(UINT indexCount, UINT startIndexLocation = 0, INT baseVertexLocation = 0);
    void DrawInstanced(UINT vertexCountPerInstance, UINT instanceCount,
        UINT StartVertexLocation = 0, UINT startInstanceLocation = 0);
    void DrawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation,
        INT baseVertexLocation, UINT startInstanceLocation);
};
}