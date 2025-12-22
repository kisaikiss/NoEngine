#pragma once
#include "PSO.h"

namespace NoEngine {
class GraphicsPSO : public PSO {
	friend class CommandContext;
public:
	/// <summary>
	/// 空のステートから開始します。
	/// </summary>
	/// <param name="name">PSOに付ける名前</param>
	GraphicsPSO(std::wstring name = L"Unnamed Graphics PSO");

    void SetBlendState(const D3D12_BLEND_DESC& blendDesc);
    void SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterizerDesc);
    void SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc);
    void SetSampleMask(UINT sampleMask);
    void SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType);
    void SetDepthTargetFormat(DXGI_FORMAT dsvFormat, UINT msaaCount = 1, UINT msaaQuality = 0);
    void SetRenderTargetFormat(DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat, UINT msaaCount = 1, UINT msaaQuality = 0);
    void SetRenderTargetFormats(UINT numRTVs, const DXGI_FORMAT* rtvFormats, DXGI_FORMAT dsvFormat, UINT msaaCount = 1, UINT msaaQuality = 0);
    void SetInputLayout(UINT numElements, const D3D12_INPUT_ELEMENT_DESC* inputElementDescs);
    void SetPrimitiveRestart(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE ibProps);

    void SetVertexShader(const void* binary, size_t size) { psoDesc_.VS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(binary), size); }
    void SetPixelShader(const void* binary, size_t size) { psoDesc_.PS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(binary), size); }
    void SetGeometryShader(const void* binary, size_t size) { psoDesc_.GS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(binary), size); }
    void SetHullShader(const void* binary, size_t size) { psoDesc_.HS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(binary), size); }
    void SetDomainShader(const void* binary, size_t size) { psoDesc_.DS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(binary), size); }

    void SetVertexShader(const D3D12_SHADER_BYTECODE& binary) { psoDesc_.VS = binary; }
    void SetPixelShader(const D3D12_SHADER_BYTECODE& binary) { psoDesc_.PS = binary; }
    void SetGeometryShader(const D3D12_SHADER_BYTECODE& binary) { psoDesc_.GS = binary; }
    void SetHullShader(const D3D12_SHADER_BYTECODE& binary) { psoDesc_.HS = binary; }
    void SetDomainShader(const D3D12_SHADER_BYTECODE& binary) { psoDesc_.DS = binary; }

    /// <summary>
    /// 検証を実行し、高速な状態ブロック比較のためのハッシュ値を計算します。
    /// </summary>
    void Finalize();

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc_;
private:
	
	std::shared_ptr<const D3D12_INPUT_ELEMENT_DESC> inputLayouts_;
};
}

