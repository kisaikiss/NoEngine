#pragma once
#include "../RenderPass.h"
#include "engine/Runtime/PipelineStateObject/GraphicsPSO.h"
#include "engine/Runtime/PipelineStateObject/RootSignature.h"

namespace NoEngine::Render {

// エディタの起動中だけ保持する表示設定。配置用スナップとは独立させる。
struct GridSettings {
    bool enabled = true;
    bool showInPlay = false;
    bool adaptive = true;
    bool showAxes = true;
    bool overlay = false;
    int plane = 0;
    float origin[3] = {};
    float spacing = 1.f;
    int majorEvery = 5;
    float radius = 100.f;
    float fadeStart = 0.65f;
    float minorWidth = 1.f;
    float majorWidth = 1.5f;
    float axisWidth = 2.f;
    float depthBias = 0.000001f;
    float minorColor[4] = { 0.35f, 0.38f, 0.42f, 0.28f };
    float majorColor[4] = { 0.5f, 0.54f, 0.6f, 0.48f };
};

class GridPass : public RenderPass {
public:
    void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resources, ECS::Registry& registry) override;
    void DrawEditor() override;
private:
    void Initialize();
    GridSettings settings_;
    RootSignature rootSignature_;
    GraphicsPSO pso_;
    GraphicsPSO overlayPso_;
    bool initialized_ = false;
};
}
