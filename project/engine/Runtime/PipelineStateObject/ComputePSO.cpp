#include "ComputePSO.h"
#include "RootSignature.h"
#include "engine/Utilities/Hash.h"
#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine {
using namespace std;

ComputePSO::ComputePSO(std::wstring name) : PSO(name) {
	ZeroMemory(&psoDesc_, sizeof(psoDesc_));
	psoDesc_.NodeMask = 1;
}

void ComputePSO::Finalize() {
    // Make sure the root signature is finalized first
    psoDesc_.pRootSignature = rootSignature_->GetSignature();
    assert(psoDesc_.pRootSignature != nullptr);

    size_t HashCode = Utility::HashState(&psoDesc_);

    ID3D12PipelineState** PSORef = nullptr;
    bool firstCompile = false;
    {
        static mutex s_HashMapMutex;
        lock_guard<mutex> CS(s_HashMapMutex);
        auto iter = sComputePSOHashMap.find(HashCode);

        // Reserve space so the next inquiry will find that someone got here first.
        if (iter == sComputePSOHashMap.end()) {
            firstCompile = true;
            PSORef = sComputePSOHashMap[HashCode].GetAddressOf();
        } else
            PSORef = iter->second.GetAddressOf();
    }

    if (firstCompile) {
        HRESULT hr = GraphicsCore::gGraphicsDevice->GetDevice()->CreateComputePipelineState(&psoDesc_, IID_PPV_ARGS(&pso_));
        if (FAILED(hr)) {
            assert(false);
        }
        sComputePSOHashMap[HashCode].Attach(pso_);
        pso_->SetName(name_.c_str());
    } else {
        while (*PSORef == nullptr)
            this_thread::yield();
        pso_ = *PSORef;
    }
}

}