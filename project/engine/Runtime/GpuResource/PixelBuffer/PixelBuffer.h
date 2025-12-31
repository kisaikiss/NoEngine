#pragma once
#include "../GpuResource.h"

namespace NoEngine {
/// <summary>
/// テクスチャリソースの基盤クラス
/// </summary>
class PixelBuffer : public GpuResource {
public:
    PixelBuffer() : width_(0), height_(0), arraySize_(0), format_(DXGI_FORMAT_UNKNOWN), bankRotation_(0) {};

    uint32_t GetWidth(void) const { return width_; }
    uint32_t GetHeight(void) const { return height_; }
    uint32_t GetDepth(void) const { return arraySize_; }
    const DXGI_FORMAT& GetFormat(void) const { return format_; }

    

protected:
    /// <summary>
    /// D3D12_RESOURCE_DESCを作成します。
    /// </summary>
    /// <param name="width">テクスチャの幅</param>
    /// <param name="height">テクスチャの高さ</param>
    /// <param name="depthOrArraySize">Depthの奥行き(2Dテクスチャなので基本は1)</param>
    /// <param name="numMips">ミップマップの数</param>
    /// <param name="format">ピクセルフォーマット</param>
    /// <param name="flags">リソースの用途に応じたフラグ</param>
    /// <returns>作成したD3D12_RESOURCE_DESC</returns>
    D3D12_RESOURCE_DESC DescribeTex2D(uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t numMips, DXGI_FORMAT format, UINT flags);

    /// <summary>
    /// リソースの関連付け
    /// </summary>
    /// <param name="name">デバッグ用の名前</param>
    /// <param name="resource">関連付けるリソース</param>
    /// <param name="currentState">現在のリソースステート</param>
    void AssociateWithResource(const std::wstring& name, ID3D12Resource* resource, D3D12_RESOURCE_STATES currentState);

    /// <summary>
    /// テクスチャリソースを作成します。
    /// </summary>
    /// <param name="device">デバイス</param>
    /// <param name="name">デバッグ用の名前</param>
    /// <param name="resourceDesc">リソースの設定</param>
    /// <param name="clearValue">クリアする値</param>
    void CreateTextureResource(ID3D12Device* device, const std::wstring& name, const D3D12_RESOURCE_DESC& resourceDesc, D3D12_CLEAR_VALUE clearValue);

    /// <summary>
    /// 指定したDXGI_FORMATのTYPELESS版を返します。
    /// </summary>
    /// <param name="format">TYPELESS版が欲しいDXGI_FORMAT</param>
    /// <returns>DXGI_FORMAT_TYPELESS</returns>
    static DXGI_FORMAT GetBaseFormat(DXGI_FORMAT format);
    static DXGI_FORMAT GetDSVFormat(DXGI_FORMAT format);
    static DXGI_FORMAT GetDepthFormat(DXGI_FORMAT format);
    static DXGI_FORMAT GetStencilFormat(DXGI_FORMAT format);
    uint32_t width_;
    uint32_t height_;
    uint32_t arraySize_;
    DXGI_FORMAT format_;

    // バンクコンフリクトを避けるのに利用します。
    uint32_t bankRotation_;

};
}
