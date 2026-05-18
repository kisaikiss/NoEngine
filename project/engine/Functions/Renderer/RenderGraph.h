#pragma once
#include "engine/Runtime/GpuResource/PixelBuffer/ColorBuffer.h"

namespace NoEngine {
// パスがリソースの利用を申告するためのクラス
class RenderGraphBuilder {
public:
    // このパスでRTVとして書き込むリソースを登録
    void WriteRenderTarget(const std::string& name) {
        outputs_.push_back(name);
    }
    // このパスでSRVとして読み込むリソースを登録
    void ReadTexture(const std::string& name) {
        inputs_.push_back(name);
    }

private:
    std::vector<std::string> inputs_;
    std::vector<std::string> outputs_;
    friend class RenderPassScheduler; // スケジューラがこの情報を回収する
};

// Execute時に、実際の ColorBuffer にアクセスするためのクラス
class RenderGraphRegistry {
public:
    void RegisterResource(const std::string& name, ColorBuffer* buffer) {
        resources_[name] = buffer;
    }
    ColorBuffer& GetColorBuffer(const std::string& name) {
        return *resources_[name];
    }
private:
    std::unordered_map<std::string, ColorBuffer*> resources_;
};

}