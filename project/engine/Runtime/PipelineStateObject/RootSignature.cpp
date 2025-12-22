#include "RootSignature.h"

#include "engine/Debug/Logger/Log.h"
#include "engine/Utilities/Hash.h"
#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine {
using namespace std;

namespace {
static std::map< size_t, Microsoft::WRL::ComPtr<ID3D12RootSignature> > sRootSignatureHashMap;
}

void RootSignature::DestroyAll() {
    sRootSignatureHashMap.clear();
}

RootSignature::RootSignature(UINT NumRootParams, UINT NumStaticSamplers) :
    finalized_(FALSE),
    numParameters_(NumRootParams) {
    Reset(NumRootParams, NumStaticSamplers);
}

void RootSignature::Reset(UINT NumRootParams, UINT NumStaticSamplers) {
    if (NumRootParams > 0)
        paramArray_.reset(new RootParameter[NumRootParams]);
    else
        paramArray_ = nullptr;
    numParameters_ = NumRootParams;

    if (NumStaticSamplers > 0)
        samplerArray_.reset(new D3D12_STATIC_SAMPLER_DESC[NumStaticSamplers]);
    else
        samplerArray_ = nullptr;
    numSamplers_ = NumStaticSamplers;
    numInitializedStaticSamplers_ = 0;
}

void RootSignature::InitStaticSampler(UINT Register, const D3D12_SAMPLER_DESC& NonStaticSamplerDesc, D3D12_SHADER_VISIBILITY Visibility) {

    assert(numInitializedStaticSamplers_ < numSamplers_);
    D3D12_STATIC_SAMPLER_DESC& StaticSamplerDesc = samplerArray_[numInitializedStaticSamplers_++];

    StaticSamplerDesc.Filter = NonStaticSamplerDesc.Filter;
    StaticSamplerDesc.AddressU = NonStaticSamplerDesc.AddressU;
    StaticSamplerDesc.AddressV = NonStaticSamplerDesc.AddressV;
    StaticSamplerDesc.AddressW = NonStaticSamplerDesc.AddressW;
    StaticSamplerDesc.MipLODBias = NonStaticSamplerDesc.MipLODBias;
    StaticSamplerDesc.MaxAnisotropy = NonStaticSamplerDesc.MaxAnisotropy;
    StaticSamplerDesc.ComparisonFunc = NonStaticSamplerDesc.ComparisonFunc;
    StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    StaticSamplerDesc.MinLOD = NonStaticSamplerDesc.MinLOD;
    StaticSamplerDesc.MaxLOD = NonStaticSamplerDesc.MaxLOD;
    StaticSamplerDesc.ShaderRegister = Register;
    StaticSamplerDesc.RegisterSpace = 0;
    StaticSamplerDesc.ShaderVisibility = Visibility;

    if (StaticSamplerDesc.AddressU == D3D12_TEXTURE_ADDRESS_MODE_BORDER ||
        StaticSamplerDesc.AddressV == D3D12_TEXTURE_ADDRESS_MODE_BORDER ||
        StaticSamplerDesc.AddressW == D3D12_TEXTURE_ADDRESS_MODE_BORDER) {
        if (
            // Transparent Black
            NonStaticSamplerDesc.BorderColor[0] == 0.0f &&
            NonStaticSamplerDesc.BorderColor[1] == 0.0f &&
            NonStaticSamplerDesc.BorderColor[2] == 0.0f &&
            NonStaticSamplerDesc.BorderColor[3] == 0.0f ||
            // Opaque Black
            NonStaticSamplerDesc.BorderColor[0] == 0.0f &&
            NonStaticSamplerDesc.BorderColor[1] == 0.0f &&
            NonStaticSamplerDesc.BorderColor[2] == 0.0f &&
            NonStaticSamplerDesc.BorderColor[3] == 1.0f ||
            // Opaque White
            NonStaticSamplerDesc.BorderColor[0] == 1.0f &&
            NonStaticSamplerDesc.BorderColor[1] == 1.0f &&
            NonStaticSamplerDesc.BorderColor[2] == 1.0f &&
            NonStaticSamplerDesc.BorderColor[3] == 1.0f) {
            Log::DebugPrint("Sampler border color does not match static sampler limitations",VerbosityLevel::kCritical);
            assert(false);
        }

        if (NonStaticSamplerDesc.BorderColor[3] == 1.0f) {
            if (NonStaticSamplerDesc.BorderColor[0] == 1.0f)
                StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
            else
                StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
        } else
            StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    }
}

void RootSignature::Finalize(const std::wstring& name, D3D12_ROOT_SIGNATURE_FLAGS Flags) {
    if (finalized_) {
        return;
    }
    assert(numInitializedStaticSamplers_ == numSamplers_);

    D3D12_ROOT_SIGNATURE_DESC RootDesc;
    RootDesc.NumParameters = numParameters_;
    RootDesc.pParameters = (const D3D12_ROOT_PARAMETER*)paramArray_.get();
    RootDesc.NumStaticSamplers = numSamplers_;
    RootDesc.pStaticSamplers = (const D3D12_STATIC_SAMPLER_DESC*)samplerArray_.get();
    RootDesc.Flags = Flags;

    descriptorTableBitMap_ = 0;
    samplerTableBitMap_ = 0;

    size_t HashCode = Utility::HashState(&RootDesc.Flags);
    HashCode = Utility::HashState(RootDesc.pStaticSamplers, numSamplers_, HashCode);

    for (UINT Param = 0; Param < numParameters_; ++Param) {
        const D3D12_ROOT_PARAMETER& RootParam = RootDesc.pParameters[Param];
        descriptorTableSize_[Param] = 0;

        if (RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
            assert(RootParam.DescriptorTable.pDescriptorRanges != nullptr);

            HashCode = Utility::HashState(RootParam.DescriptorTable.pDescriptorRanges,
                RootParam.DescriptorTable.NumDescriptorRanges, HashCode);

            // We keep track of sampler descriptor tables separately from CBV_SRV_UAV descriptor tables
            if (RootParam.DescriptorTable.pDescriptorRanges->RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
                samplerTableBitMap_ |= (1 << Param);
            else
                descriptorTableBitMap_ |= (1 << Param);

            for (UINT TableRange = 0; TableRange < RootParam.DescriptorTable.NumDescriptorRanges; ++TableRange)
                descriptorTableSize_[Param] += RootParam.DescriptorTable.pDescriptorRanges[TableRange].NumDescriptors;
        } else
            HashCode = Utility::HashState(&RootParam, 1, HashCode);
    }

    ID3D12RootSignature** RSRef = nullptr;
    bool firstCompile = false;
    {
        static mutex s_HashMapMutex;
        lock_guard<mutex> CS(s_HashMapMutex);
        auto iter = sRootSignatureHashMap.find(HashCode);

        // Reserve space so the next inquiry will find that someone got here first.
        if (iter == sRootSignatureHashMap.end()) {
            RSRef = sRootSignatureHashMap[HashCode].GetAddressOf();
            firstCompile = true;
        } else
            RSRef = iter->second.GetAddressOf();
    }

    if (firstCompile) {
        Microsoft::WRL::ComPtr<ID3DBlob> pOutBlob, pErrorBlob;

        HRESULT hr = D3D12SerializeRootSignature(&RootDesc, D3D_ROOT_SIGNATURE_VERSION_1, pOutBlob.GetAddressOf(), pErrorBlob.GetAddressOf());
        if (FAILED(hr)) {
            assert(false);
        }

        hr = (GraphicsCore::gGraphicsDevice->GetDevice()->CreateRootSignature(0, pOutBlob->GetBufferPointer(), pOutBlob->GetBufferSize(),
            IID_PPV_ARGS(&signature_)));
        if (FAILED(hr)) {
            assert(false);
        }

        signature_->SetName(name.c_str());

        sRootSignatureHashMap[HashCode].Attach(signature_);
        assert(*RSRef == signature_);
    } else {
        while (*RSRef == nullptr)
            this_thread::yield();
        signature_ = *RSRef;
    }

    finalized_ = true;
}
}