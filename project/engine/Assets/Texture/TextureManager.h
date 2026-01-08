#pragma once
#include "externals/DirectXTex/DirectXTex.h"
#include "Texture.h"
#include "engine/Runtime/Graphics/GraphicsCommon.h"

namespace NoEngine {
// ToDo : Texture周りはコピペが多く、理解が薄い部分が多いです。全体的に理解し、リファクタリングをすべきです。
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

private:
	static DirectX::ScratchImage LordTexture(const std::string& filePath);
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

    // SRV記述子ハンドルを取得します。参照が無効な場合、
    // 有効な記述子ハンドル（フォールバックで指定）を返します。
    D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const;

    // テクスチャポインタを取得します。クライアントは null ポインタを参照しない責任があります。
    const Texture* Get(void) const;

    const Texture* operator->(void) const;

private:
    ManagedTexture* ref_;
};
}