#pragma once
#include "externals/DirectXTex/DirectXTex.h"
#include "Texture.h"
#include "engine/Runtime/Graphics/GraphicsCommon.h"

namespace NoEngine {
class ManagedTexture;
class TextureRef;

class TextureManager {
public:

    static void Initialize(const std::wstring& TextureRoot);
    static void Shutdown(void);
    static void DestroyTexture(std::wstring mapKey);
    static TextureRef LoadTextureFile(const std::wstring& filePath, Graphics::eDefaultTexture fallback = Graphics::eDefaultTexture::kMagenta2D, bool sRGB = false);
    static TextureRef LoadTextureFile(const std::string& filePath, Graphics::eDefaultTexture fallback = Graphics::eDefaultTexture::kMagenta2D, bool sRGB = false);

    static TextureRef LoadCovertTexture(const std::wstring& filePath, Graphics::eDefaultTexture fallback = Graphics::kMagenta2D, bool sRGB = false);
    static TextureRef LoadCovertTexture(const std::string& filePath, Graphics::eDefaultTexture fallback = Graphics::kMagenta2D, bool sRGB = false);

    // 実行時に生成したCPUピクセルデータからテクスチャを作成する。
    // フォントアトラスなど、ファイルを経由しない画像をキャッシュに載せたい場合に使う。
    // key は sTextureCache 内で一意な識別子(通常のファイルパスと衝突しない接頭辞を付けること)。
    static TextureRef CreateFromPixels(const std::wstring& key, uint32_t width, uint32_t height,
        DXGI_FORMAT format, const void* pixelData, size_t rowPitchBytes);

   // CreateFromPixelsで作成済みのテクスチャの中身を差し替える。
   // keyが未登録の場合は何もせずfalseを返す。
   // 注意: 前フレームの描画コマンドが同じGPUリソースを参照している可能性があるため、
   // 毎フレームの呼び出しは避け、フォントへの新規文字追加のような低頻度イベント向けに使うこと。
    static bool UpdatePixels(const std::wstring& key, uint32_t width, uint32_t height,
        DXGI_FORMAT format, const void* pixelData, size_t rowPitchBytes);
private:
    static DirectX::ScratchImage LoadTexture(const std::string& filePath);
    static ManagedTexture* FindOrLoadTexture(const std::wstring& fileName, Graphics::eDefaultTexture fallback, bool forceSRGB = false);
};

class TextureRef {
public:

    TextureRef(const TextureRef& ref);
    TextureRef(ManagedTexture* tex = nullptr);
    ~TextureRef();

    void operator= (std::nullptr_t);
    void operator= (TextureRef& rhs);
    TextureRef& operator=(const TextureRef& rhs);
    TextureRef& operator=(TextureRef&& rhs) noexcept;

    // これが有効なテクスチャ（正常にロードされたもの）を指していることを確認します。
    bool IsValid() const;

    /// <summary>
    /// 画像の横幅を返します
    /// </summary>
    /// <returns>uint32_t型の画像の横幅</returns>
    uint32_t GetWidth();

    /// <summary>
    /// 画像の縦幅を返します
    /// </summary>
    /// <returns>uint32_t型の画像の縦幅</returns>
    uint32_t GetHeight();

    // SRV記述子ハンドルを取得します。参照が無効な場合、
    // 有効な記述子ハンドル（フォールバックで指定）を返します。
    D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const;

    // テクスチャポインタを取得します。クライアントは null ポインタを参照しない責任があります。
    const Texture* Get(void) const;

    const Texture* operator->(void) const;

    bool operator<(const TextureRef& other) const {
        return ref_ < other.ref_; // SpritePassでテクスチャをまとめて描画するために使用します。
    }

    bool operator==(const TextureRef& other) const {
        return ref_ == other.ref_;
    }

private:
    ManagedTexture* ref_;
};
}