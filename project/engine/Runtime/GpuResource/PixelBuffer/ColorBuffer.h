#pragma once
#include "PixelBuffer.h"
#include "engine/Math/Color/Color.h"
namespace NoEngine {
/// <summary>
/// バックバッファやオフスクリーン用のリソース
/// </summary>
class ColorBuffer : public PixelBuffer{
public:
    ColorBuffer(Math::Color clearColor = Math::Color(0x66cdaaff)) :
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

    /// <summary>
    /// 指定した名前、サイズ、ミップ数、フォーマット、および省略可能な GPU 仮想アドレスでリソースを作成します。
    /// </summary>
    /// <param name="name">作成するリソースの名前（const std::wstring&）。</param>
    /// <param name="width">リソースの幅（ピクセル単位）。</param>
    /// <param name="height">リソースの高さ（ピクセル単位）。</param>
    /// <param name="numMips">生成するミップマップレベルの数。</param>
    /// <param name="format">リソースのピクセル/データフォーマット（DXGI_FORMAT）。</param>
    void Create(const std::wstring& name, uint32_t width, uint32_t height, uint32_t numMips,
        DXGI_FORMAT format);

    /// <summary>
   /// 指定した名前、サイズ、ミップ数、フォーマット、および省略可能な GPU 仮想アドレスでリソースを作成します。
   /// </summary>
   /// <param name="name">作成するリソースの名前（const std::wstring&）。</param>
   /// <param name="width">リソースの幅（ピクセル単位）。</param>
   /// <param name="height">リソースの高さ（ピクセル単位）。</param>
   /// <param name="depthOrArraySize">Depthの奥行き</param>
   /// <param name="numMips">生成するミップマップレベルの数。</param>
   /// <param name="format">リソースのピクセル/データフォーマット（DXGI_FORMAT）。</param>
    void Create(const std::wstring& name, uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t numMips,
        DXGI_FORMAT format);

    void CreateImGuiSRV();
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetImGuiSRV(void) const { return imguiSRV_; }

    Math::Color& GetClearColor() { return clearColor_; }

    const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV(void) const { return srvHandle_; }
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetRTV(void) const { return rtvHandle_; }
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetUAV(void) const { return uavHandle_[0]; }
private:
    Math::Color clearColor_;
    D3D12_CPU_DESCRIPTOR_HANDLE srvHandle_;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_;
    std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 12> uavHandle_;
    uint32_t numMipMaps_; // number of texture sublevels
    uint32_t fragmentCount_;
    uint32_t sampleCount_;

    // ImGui用のCPU_DESCRIPTTOR_HANDLE
    D3D12_CPU_DESCRIPTOR_HANDLE imguiSRV_;

    D3D12_RESOURCE_FLAGS CombineResourceFlags(void) const {
        D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE;

        if (Flags == D3D12_RESOURCE_FLAG_NONE && fragmentCount_ == 1)
            Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

        return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | Flags;
    }

    // 1x1 に縮小するために必要なテクスチャレベル数を計算します。
    // _BitScanReverse を使用して、最も高いセットビットを見つけます。各次元は
    // 半分に縮小され、ビットは切り捨てられます。 256 (0x100) には 9 つのミップレベルがあり、これは 511 (0x1FF) と同じです。
    static inline uint32_t ComputeNumMips(uint32_t Width, uint32_t Height) {
        uint32_t HighBit;
        _BitScanReverse((unsigned long*)&HighBit, Width | Height);
        return HighBit + 1;
    }


    void CreateDerivedViews(ID3D12Device* Device, DXGI_FORMAT Format, uint32_t ArraySize, uint32_t NumMips = 1);
};
}
