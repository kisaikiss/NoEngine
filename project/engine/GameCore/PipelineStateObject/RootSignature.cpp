#include "RootSignature.h"

#include "engine/Debug/Logger/Log.h"

namespace NoEngine {
using namespace std;
NoEngine::RootSignature::RootSignature(UINT NumRootParams, UINT NumStaticSamplers) :
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

void RootSignature::Finalize(/*const std::wstring& name, D3D12_ROOT_SIGNATURE_FLAGS Flags*/) {
    if (finalized_) {
        return;
    }
      
    finalized_ = true;
}
}