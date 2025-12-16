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

    void SetRenderTargets(UINT NumRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[]);
    void SetRenderTargets(UINT NumRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[], D3D12_CPU_DESCRIPTOR_HANDLE DSV);
    void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE RTV) { SetRenderTargets(1, &RTV); }
private:

};
}