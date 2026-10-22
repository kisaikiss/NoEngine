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
    // 先にルートシグネチャを確定しておく
    psoDesc_.pRootSignature = rootSignature_->GetSignature();
    assert(psoDesc_.pRootSignature != nullptr);

    size_t HashCode = Utility::HashState(&psoDesc_);

    ID3D12PipelineState** PSORef = nullptr;
    bool firstCompile = false;
    {
        static mutex s_HashMapMutex;
        lock_guard<mutex> CS(s_HashMapMutex);
        auto iter = sComputePSOHashMap.find(HashCode);

        // 次の問い合わせで先行処理が見つかるよう、領域を確保する
        if (iter == sComputePSOHashMap.end()) {
            firstCompile = true;
            PSORef = sComputePSOHashMap[HashCode].GetAddressOf();
        } else
            PSORef = iter->second.GetAddressOf();
    }

    if (firstCompile) {
        HRESULT hr = GraphicsCore::sGraphicsDevice->GetDevice()->CreateComputePipelineState(&psoDesc_, IID_PPV_ARGS(&pso_));
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
