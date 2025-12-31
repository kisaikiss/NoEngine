#pragma once
#include "PixelBuffer.h"
namespace NoEngine {
class DepthBuffer : public PixelBuffer {
public:
    DepthBuffer(float ClearDepth = 0.0f, uint8_t ClearStencil = 0)
        : clearDepth_(ClearDepth), clearStencil_(ClearStencil) {
        dsvHandle_[0].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        dsvHandle_[1].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        dsvHandle_[2].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        dsvHandle_[3].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        depthSRVHandle_.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        stencilSRVHandle_.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
    }

    // 深度バッファを作成します。
    void Create(const std::wstring& name, uint32_t width, uint32_t height, DXGI_FORMAT format);
    void Create(const std::wstring& name, uint32_t width, uint32_t height, uint32_t numSamples, DXGI_FORMAT format);


    // 事前に作成されたCPUディスクリプタハンドルのゲッタ
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV() const { return dsvHandle_[0]; }
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV_DepthReadOnly() const { return dsvHandle_[1]; }
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV_StencilReadOnly() const { return dsvHandle_[2]; }
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV_ReadOnly() const { return dsvHandle_[3]; }
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetDepthSRV() const { return depthSRVHandle_; }
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetStencilSRV() const { return stencilSRVHandle_; }

    float GetClearDepth() const { return clearDepth_; }
    uint8_t GetClearStencil() const { return clearStencil_; }
protected:
    void CreateDerivedViews(ID3D12Device* device, DXGI_FORMAT format);

    float clearDepth_;
    uint8_t clearStencil_;
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_[4];
    D3D12_CPU_DESCRIPTOR_HANDLE depthSRVHandle_;
    D3D12_CPU_DESCRIPTOR_HANDLE stencilSRVHandle_;
};
}
