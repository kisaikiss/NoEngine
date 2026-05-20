#pragma once
#include "engine/Runtime/GpuResource/PixelBuffer/ColorBuffer.h"

namespace NoEngine {
// パスがリソースの利用を申告するためのクラス
class RenderGraphBuilder {
public:
    struct OutputDesc {
        std::string name;
        bool autoClear; // trueだと描画前にクリアする
    };

    // このパスでRTVとして書き込むリソースを登録
    void WriteRenderTarget(const std::string& name, bool autoClear = false) {
        outputs_.push_back({ name, autoClear });
    }
    // このパスでSRVとして読み込むリソースを登録
    void ReadTexture(const std::string& name) {
        inputs_.push_back(name);
    }
private:
   
    std::vector<std::string> inputs_;
    std::vector<OutputDesc> outputs_;
    friend class RenderPassScheduler; // スケジューラがこの情報を回収する
};

// Execute時に、実際の ColorBuffer にアクセスするためのクラス
class RenderGraphRegistry {
public:

    ColorBuffer* Create(const std::string& name, uint32_t width, uint32_t height, DXGI_FORMAT format);
    ColorBuffer* Create(const std::string& name, float width, float height, DXGI_FORMAT format);

    const ColorBuffer& GetColorBuffer(const std::string& name) const{
        if (!resources_.contains(name))assert(false);
        return resources_.find(name)->second;
    }
private:
    std::unordered_map<std::string, ColorBuffer> resources_;

    ColorBuffer* GetColorBufferPointer(const std::string& name) {
        if (!resources_.contains(name))assert(false);
        return &resources_.find(name)->second;
    }

    friend class RenderPassScheduler; // スケジューラがこの情報を回収する
};

}