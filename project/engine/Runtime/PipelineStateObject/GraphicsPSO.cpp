#include "GraphicsPSO.h"

#include "RootSignature.h"
#include "engine/Functions/Debug/Logger/Log.h"
#include "engine/Utilities/Hash.h"
#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine {
GraphicsPSO::GraphicsPSO(std::wstring name) : PSO(name) {
	// PSODescの中身を0で埋めます。
	ZeroMemory(&psoDesc_, sizeof(psoDesc_));
	// PSODescのメンバ変数の初期値を設定します。
	psoDesc_.NodeMask = 1;
	psoDesc_.SampleMask = 0xFFFFFFFFu;
	psoDesc_.SampleDesc.Count = 1;
	psoDesc_.InputLayout.NumElements = 0;
}

void GraphicsPSO::SetBlendState(const D3D12_BLEND_DESC& blendDesc) {
	psoDesc_.BlendState = blendDesc;
}

void GraphicsPSO::SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterizerDesc) {
	psoDesc_.RasterizerState = rasterizerDesc;
}

void GraphicsPSO::SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc) {
	psoDesc_.DepthStencilState = depthStencilDesc;
}

void GraphicsPSO::SetSampleMask(UINT sampleMask) {
	psoDesc_.SampleMask = sampleMask;
}

void GraphicsPSO::SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType) {
	psoDesc_.PrimitiveTopologyType = topologyType;
}

void GraphicsPSO::SetDepthTargetFormat(DXGI_FORMAT dsvFormat, UINT msaaCount, UINT msaaQuality) {
	SetRenderTargetFormats(0, nullptr, dsvFormat, msaaCount, msaaQuality);
}

void GraphicsPSO::SetRenderTargetFormat(DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat, UINT msaaCount, UINT msaaQuality) {
	SetRenderTargetFormats(1, &rtvFormat, dsvFormat, msaaCount, msaaQuality);
}

void GraphicsPSO::SetRenderTargetFormats(UINT numRTVs, const DXGI_FORMAT* rtvFormats, DXGI_FORMAT dsvFormat, UINT msaaCount, UINT msaaQuality) {
	if (numRTVs != 0 && rtvFormats == nullptr) {
		Log::DebugPrint("Null format array conflicts with non-zero length", VerbosityLevel::kCritical);
		assert(false);
	}

	for (UINT i = 0; i < numRTVs; ++i) {
		assert(rtvFormats[i] != DXGI_FORMAT_UNKNOWN);
		psoDesc_.RTVFormats[i] = rtvFormats[i];
	}

	for (UINT i = numRTVs; i < psoDesc_.NumRenderTargets; ++i) {
		psoDesc_.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
	}
	psoDesc_.NumRenderTargets = numRTVs;
	psoDesc_.DSVFormat = dsvFormat;
	psoDesc_.SampleDesc.Count = msaaCount;
	psoDesc_.SampleDesc.Quality = msaaQuality;
}

void GraphicsPSO::SetInputLayout(const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputElements) {
	SetInputLayout(static_cast<UINT>(inputElements.size()), inputElements.data());
}

void GraphicsPSO::SetInputLayout(UINT numElements, const D3D12_INPUT_ELEMENT_DESC* inputElementDescs) {
	psoDesc_.InputLayout.NumElements = numElements;
	if (numElements > 0) {
		D3D12_INPUT_ELEMENT_DESC* NewElements = (D3D12_INPUT_ELEMENT_DESC*)malloc(sizeof(D3D12_INPUT_ELEMENT_DESC) * numElements);
		memcpy(NewElements, inputElementDescs, numElements * sizeof(D3D12_INPUT_ELEMENT_DESC));
		inputLayouts_.reset((const D3D12_INPUT_ELEMENT_DESC*)NewElements);
	} else {
		inputLayouts_ = nullptr;
	}

}

void GraphicsPSO::SetPrimitiveRestart(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE ibProps) {
	psoDesc_.IBStripCutValue = ibProps;
}

void GraphicsPSO::Finalize() {

	// Make sure the root signature is finalized first
	psoDesc_.pRootSignature = rootSignature_->GetSignature();
	assert(psoDesc_.pRootSignature != nullptr);

	psoDesc_.InputLayout.pInputElementDescs = nullptr;
	size_t HashCode = Utility::HashState(&psoDesc_);
	HashCode = Utility::HashState(inputLayouts_.get(), psoDesc_.InputLayout.NumElements, HashCode);
	psoDesc_.InputLayout.pInputElementDescs = inputLayouts_.get();

	ID3D12PipelineState** PSORef = nullptr;
	bool firstCompile = true;
	{
		static std::mutex s_HashMapMutex;
		std::lock_guard<std::mutex> CS(s_HashMapMutex);
		auto iter = sGraphicsPSOHashMap.find(HashCode);

		// Reserve space so the next inquiry will find that someone got here first.
		if (iter == sGraphicsPSOHashMap.end()) {
			firstCompile = true;
			PSORef = sGraphicsPSOHashMap[HashCode].GetAddressOf();
		} else
			PSORef = iter->second.GetAddressOf();
	}

	if (firstCompile) {
		assert(psoDesc_.DepthStencilState.DepthEnable != (psoDesc_.DSVFormat == DXGI_FORMAT_UNKNOWN));
		HRESULT hr = GraphicsCore::gGraphicsDevice->GetDevice()->CreateGraphicsPipelineState(&psoDesc_, IID_PPV_ARGS(&pso_));
		if (FAILED(hr)) {
			assert(false);
		}
		sGraphicsPSOHashMap[HashCode].Attach(pso_);
		pso_->SetName(name_.c_str());
	} else {
		while (*PSORef == nullptr)
			std::this_thread::yield();
		pso_ = *PSORef;
	}
}

}