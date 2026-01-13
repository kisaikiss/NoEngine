#include "RootSignature.h"

#include "engine/Functions/Debug/Logger/Log.h"
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

            // サンプラー記述子テーブルはCBV_SRV_UAV記述子テーブルとは別に管理します。
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
            NoEngine::Log::DebugPrint((char*)pErrorBlob->GetBufferPointer(), VerbosityLevel::kCritical);
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
void RootSignature::Dump() const {
    std::string dump;
    dump += ("==== RootSignature Dump ====\n");

    for (UINT i = 0; i < numParameters_; ++i) {
        const auto& p = paramArray_[i];

        dump += "RootParam[" + std::to_string(i) + "]:";
     
        switch (p.rootParameter_.ParameterType) {
        case D3D12_ROOT_PARAMETER_TYPE_CBV:
            dump += ("CBV\n");
            dump += ("  Register: " + std::to_string(p.rootParameter_.Descriptor.ShaderRegister));
            dump += "\n";
            dump += ("  Space   : " + std::to_string(p.rootParameter_.Descriptor.RegisterSpace));
            dump += "\n";
            break;

        case D3D12_ROOT_PARAMETER_TYPE_SRV:
            dump += ("SRV\n");
            dump += ("  Register: " + std::to_string(p.rootParameter_.Descriptor.ShaderRegister));
            dump += "\n";
            dump += ("  Space   : " + std::to_string(p.rootParameter_.Descriptor.RegisterSpace));
            dump += "\n";
            break;

        case D3D12_ROOT_PARAMETER_TYPE_UAV:
            dump += ("UAV\n");
            dump += ("  Register: " + std::to_string(p.rootParameter_.Descriptor.ShaderRegister));
            dump += "\n";
            dump += ("  Space   : " + std::to_string(p.rootParameter_.Descriptor.RegisterSpace));
            dump += "\n";
            break;

        case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
            dump += ("32BIT_CONSTANTS\n");
            dump += ("  Register: " + std::to_string(p.rootParameter_.Constants.ShaderRegister));
            dump += "\n";
            dump += ("  Space   : " + std::to_string(p.rootParameter_.Constants.RegisterSpace));
            dump += "\n";
            dump += ("  Num32BitValues" + std::to_string(p.rootParameter_.Constants.Num32BitValues));
            dump += "\n";
            break;

        case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
            dump += ("DESCRIPTOR_TABLE\n");
            dump += ("  NumRanges: " + std::to_string(p.rootParameter_.DescriptorTable.NumDescriptorRanges));
            dump += "\n";

            for (UINT r = 0; r < p.rootParameter_.DescriptorTable.NumDescriptorRanges; ++r) {
                const auto& range = p.rootParameter_.DescriptorTable.pDescriptorRanges[r];
                dump += ("    Range[" + std::to_string(r) + "]\n");
                dump += ("      Type       : " + std::to_string(range.RangeType));
                dump += "\n";
                dump += ("      BaseReg    : " + std::to_string(range.BaseShaderRegister));
                dump += "\n";
                dump += ("      Count      : " + std::to_string(range.NumDescriptors));
                dump += "\n";
                dump += ("      Space      : " + std::to_string(range.RegisterSpace));
                dump += "\n";
            }
            break;

        default:
            dump += ("UNKNOWN\n");
            break;
        }
    }

    dump += ("==== End RootSignature Dump ====");
    Log::DebugPrint(dump);

}
}