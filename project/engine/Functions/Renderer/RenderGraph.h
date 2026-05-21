#pragma once
#include "engine/Runtime/GpuResource/PixelBuffer/ColorBuffer.h"
#include "engine/Runtime/GpuResource/PixelBuffer/DepthBuffer.h"

namespace NoEngine {
// パスがリソースの利用を申告するためのクラス
class RenderGraphBuilder {
public:
    struct OutputDesc {
        std::string name;
        bool autoClear = false; // trueだと描画前にクリアする
    };

    // このパスでRTVとして書き込むリソースを登録
    void WriteRenderTarget(const std::string& name, bool autoClear = false) {
        outputs_.push_back({ name, autoClear });
    }

    void WriteDepthStencil(const std::string& name, bool autoClear = false) {
        depthOutput_ = { name, autoClear };
        hasDepthOutput_ = true;
    }

    // このパスでSRVとして読み込むリソースを登録
    void ReadTexture(const std::string& name) {
        inputs_.push_back(name);
    }
private:
   
    std::vector<std::string> inputs_;
    std::vector<OutputDesc> outputs_;
    OutputDesc depthOutput_;
    bool hasDepthOutput_ = false;
    friend class RenderPassScheduler; // スケジューラがこの情報を回収する
};

// Execute時に、実際の ColorBuffer にアクセスするためのクラス
class RenderGraphRegistry {
public:

    ColorBuffer* CreateColorBuffer(const std::string& name, uint32_t width, uint32_t height, DXGI_FORMAT format);
    ColorBuffer* CreateColorBuffer(const std::string& name, float width, float height, DXGI_FORMAT format);
    DepthBuffer* CreateDepthBuffer(const std::string& name, uint32_t width, uint32_t height, DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT);
    DepthBuffer* CreateDepthBuffer(const std::string& name, float width, float height, DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT);

    const ColorBuffer& GetColorBuffer(const std::string& name) const{
        if (!colorBuffers_.contains(name))assert(false);
        return colorBuffers_.find(name)->second;
    }

    const DepthBuffer& GetDepthBuffer(const std::string& name) const {
        if (!depthBuffers_.contains(name))assert(false);
        return depthBuffers_.find(name)->second;
    }
private:
    std::unordered_map<std::string, ColorBuffer> colorBuffers_;
    std::unordered_map<std::string, DepthBuffer> depthBuffers_;

    ColorBuffer* GetColorBufferPointer(const std::string& name) {
        if (!colorBuffers_.contains(name))assert(false);
        auto it = colorBuffers_.find(name);
        return it != colorBuffers_.end() ? &it->second : nullptr;
    }

    DepthBuffer* GetDepthBufferPointer(const std::string& name) {
        if (!depthBuffers_.contains(name))assert(false);
        auto it = depthBuffers_.find(name);
        return it != depthBuffers_.end() ? &it->second : nullptr;
    }

    friend class RenderPassScheduler; // スケジューラがこの情報を回収する
};

}