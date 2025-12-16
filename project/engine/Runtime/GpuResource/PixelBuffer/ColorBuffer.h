#pragma once
#include "PixelBuffer.h"
#include "engine/Math/Color/Color.h"
namespace NoEngine {
/// <summary>
/// バックバッファやオフスクリーン用のリソース
/// </summary>
class ColorBuffer : public PixelBuffer{
public:
    ColorBuffer(Color clearColor = Color(0x66cdaaff)) :
        clearColor_(clearColor), numMipMaps_(0), fragmentCount_(1), sampleCount_(1) {
        srvHandle_.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        rtvHandle_.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        D3D12_CPU_DESCRIPTOR_HANDLE uavHandle;
        uavHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        uavHandle_.fill(uavHandle);
    }

    /// <summary>
    /// スワップチェーンのGetBuffer()で取得できるリソースからColorBufferを作成します。
    /// </summary>
    /// <param name="name">デバッグ用の名前</param>
    /// <param name="baseResource">スワップチェーンのGetBuffer()より取得できるリソース</param>
    void CreateFromSwapChain(const std::wstring& name, ID3D12Resource* baseResource);

    const Color& GetClearColor() { return clearColor_; }

    const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV(void) const { return srvHandle_; }
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetRTV(void) const { return rtvHandle_; }
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetUAV(void) const { return uavHandle_[0]; }
private:
    Color clearColor_;
    D3D12_CPU_DESCRIPTOR_HANDLE srvHandle_;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_;
    std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 12> uavHandle_;
    uint32_t numMipMaps_; // number of texture sublevels
    uint32_t fragmentCount_;
    uint32_t sampleCount_;
};
}
