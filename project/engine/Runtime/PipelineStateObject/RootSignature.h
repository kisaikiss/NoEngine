#pragma once
#include "RootParameter.h"

namespace NoEngine {
class RootSignature {
    friend class DynamicDescriptorHeap;
public:
    RootSignature(UINT NumRootParams = 0, UINT NumStaticSamplers = 0);

    ~RootSignature() = default;

    static void DestroyAll(void);

    void Reset(UINT NumRootParams, UINT NumStaticSamplers = 0);

    RootParameter& operator[] (size_t EntryIndex) {
        assert(EntryIndex < numParameters_);
        return paramArray_.get()[EntryIndex];
    }

    const RootParameter& operator[] (size_t EntryIndex) const {
        assert(EntryIndex < numParameters_);
        return paramArray_.get()[EntryIndex];
    }

    void InitStaticSampler(UINT Register, const D3D12_SAMPLER_DESC& NonStaticSamplerDesc,
        D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL);

    void Finalize(const std::wstring& name, D3D12_ROOT_SIGNATURE_FLAGS Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ID3D12RootSignature* GetSignature() const { return signature_; }

    void Dump() const;

protected:

    BOOL finalized_;
    UINT numParameters_;
    UINT numSamplers_;
    UINT numInitializedStaticSamplers_;
    uint32_t descriptorTableBitMap_;		// 非サンプラディスクリプタテーブルであるルートパラメータには1ビットが設定されます
    uint32_t samplerTableBitMap_;			// サンプラーディスクリプタテーブルであるルートパラメータに1ビットが設定されます
    uint32_t descriptorTableSize_[16];		// 非サンプラディスクリプタテーブルはディスクリプタの数を知る必要があります
    std::unique_ptr<RootParameter[]> paramArray_;
    std::unique_ptr<D3D12_STATIC_SAMPLER_DESC[]> samplerArray_;
    ID3D12RootSignature* signature_;

};

}
