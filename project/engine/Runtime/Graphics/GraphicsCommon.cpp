#include "GraphicsCommon.h"

#include "engine/Assets/Texture/Texture.h"

namespace NoEngine {
namespace Graphics {

namespace {
NoEngine::Texture sDefaultTextures[kNumDefaultTextures];
}

D3D12_CPU_DESCRIPTOR_HANDLE GetDefaultTexture(eDefaultTexture texID) {
    assert(texID < kNumDefaultTextures);
    return sDefaultTextures[texID].GetSRV();
}

void InitializeCommonState(void) {
    uint32_t MagentaPixel = 0xFFFF00FF;
    sDefaultTextures[kMagenta2D].Create2D(4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &MagentaPixel);
    uint32_t BlackOpaqueTexel = 0xFF000000;
    sDefaultTextures[kBlackOpaque2D].Create2D(4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &BlackOpaqueTexel);
    uint32_t BlackTransparentTexel = 0x00000000;
    sDefaultTextures[kBlackTransparent2D].Create2D(4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &BlackTransparentTexel);
    uint32_t WhiteOpaqueTexel = 0xFFFFFFFF;
    sDefaultTextures[kWhiteOpaque2D].Create2D(4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &WhiteOpaqueTexel);
    uint32_t WhiteTransparentTexel = 0x00FFFFFF;
    sDefaultTextures[kWhiteTransparent2D].Create2D(4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &WhiteTransparentTexel);
    uint32_t FlatNormalTexel = 0x00FF8080;
    sDefaultTextures[kDefaultNormalMap].Create2D(4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &FlatNormalTexel);
    //uint32_t BlackCubeTexels[6] = {};
    //sDefaultTextures[kBlackCubeMap].CreateCube(4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, BlackCubeTexels);
}

void DestroyCommonState(void) {
    for (uint32_t i = 0; i < kNumDefaultTextures; ++i)
        sDefaultTextures[i].Destroy();

}

}
}
