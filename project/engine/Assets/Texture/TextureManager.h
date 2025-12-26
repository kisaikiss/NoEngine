#pragma once
#include "externals/DirectXTex/DirectXTex.h"
#include "Texture.h"

namespace NoEngine {

class ManagedTexture;

class TextureManager {
public:
	
	static void Initialize(const std::wstring& TextureRoot);
	static void Shutdown(void);
    static void DestroyTexture(std::wstring mapKey);
private:
	static DirectX::ScratchImage LordTexture(const std::string& filePath);
};

class TextureRef {
public:

    TextureRef(const TextureRef& ref);
    TextureRef(ManagedTexture* tex = nullptr);
    ~TextureRef();

    void operator= (std::nullptr_t);
    void operator= (TextureRef& rhs);

    // Check that this points to a valid texture (which loaded successfully)
    bool IsValid() const;

    // Gets the SRV descriptor handle.  If the reference is invalid,
    // returns a valid descriptor handle (specified by the fallback)
    D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const;

    // Get the texture pointer.  Client is responsible to not dereference
    // null pointers.
    const Texture* Get(void) const;

    const Texture* operator->(void) const;

private:
    ManagedTexture* ref_;
};
}