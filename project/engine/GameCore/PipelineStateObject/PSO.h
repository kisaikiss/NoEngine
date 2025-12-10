#pragma once
namespace NoEngine {
// 前方宣言

class RootSignature;

/// <summary>
/// パイプラインステートオブジェクトの基底クラス
/// </summary>
class PSO {
public:
	PSO(std::wstring name) : name_(name), rootSignature_(nullptr), pso_(nullptr) {}

	static void DestroyAll(void);

	void SetRootSignature(const RootSignature& BindMappings) {
		rootSignature_ = &BindMappings;
	}

	const RootSignature& GetRootSignature(void) const {
		assert(rootSignature_ != nullptr);
		return *rootSignature_;
	}

	ID3D12PipelineState* GetPipelineStateObject(void) const { return pso_; }

protected:

	std::wstring name_;

	const RootSignature* rootSignature_;

	ID3D12PipelineState* pso_;

	static std::unordered_map< size_t, Microsoft::WRL::ComPtr<ID3D12PipelineState> > sGraphicsPSOHashMap;
	static std::unordered_map< size_t, Microsoft::WRL::ComPtr<ID3D12PipelineState> > sComputePSOHashMap;

};


}