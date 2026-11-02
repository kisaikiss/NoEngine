#include "stdafx.h"
#include "GridPass.h"
#include "engine/Editor/Panels/EditorWindowManager.h"
#include "engine/Functions/Shader/ShaderModule.h"
#include "engine/Functions/ECS/Component/Common/CameraComponent.h"
#include "engine/Functions/ECS/System/PlayModeController.h"
#include <algorithm>

namespace NoEngine::Render {
namespace {
// HLSLの定数バッファと同じ16バイト単位で配置する。
struct GridConstants {
    Math::Matrix4x4 viewProjection;
    float originSpacing[4];
    float centerRadius[4];
    float widthsMajor[4];
    float options[4];
    float minorColor[4];
    float majorColor[4];
    float cameraPosition[4];
};
static_assert(sizeof(GridConstants) == 176);
}

void GridPass::Initialize() {
    ShaderModule vs(ShaderStage::Vertex, L"resources/engine/Shaders/Grid.VS.hlsl", L"vs_6_0");
    ShaderModule ps(ShaderStage::Pixel, L"resources/engine/Shaders/Grid.PS.hlsl", L"ps_6_0");
    std::vector<ShaderReflection> reflections = { vs.GetReflection(), ps.GetReflection() };
    RootSignatureBuilder::BuildFromReflection(reflections, rootSignature_, "Editor Grid PSO");
    D3D12_RASTERIZER_DESC rasterizer{};
    rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizer.CullMode = D3D12_CULL_MODE_NONE;
    rasterizer.DepthClipEnable = TRUE;
    D3D12_BLEND_DESC blend{};
    auto& target = blend.RenderTarget[0];
    target.BlendEnable = TRUE;
    target.SrcBlend = D3D12_BLEND_SRC_ALPHA;
    target.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    target.BlendOp = D3D12_BLEND_OP_ADD;
    target.SrcBlendAlpha = D3D12_BLEND_ONE;
    target.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
    target.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    target.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    D3D12_DEPTH_STENCIL_DESC depth{};
    depth.DepthEnable = TRUE;
    depth.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    depth.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    auto build = [&](GraphicsPSO& pso, bool overlay) {
        // DSVを使用するPSOでは深度テストを有効に保ち、透過表示は比較を常時通過させる。
        depth.DepthFunc = overlay ? D3D12_COMPARISON_FUNC_ALWAYS : D3D12_COMPARISON_FUNC_LESS_EQUAL;
        pso.SetRootSignature(rootSignature_);
        pso.SetRasterizerState(rasterizer);
        pso.SetBlendState(blend);
        pso.SetDepthStencilState(depth);
        pso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
        pso.SetInputLayout(0, nullptr);
        pso.SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_D24_UNORM_S8_UINT);
        pso.SetVertexShader(vs.GetBytecode());
        pso.SetPixelShader(ps.GetBytecode());
        pso.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
        pso.Finalize();
    };
    build(pso_, false);
    build(overlayPso_, true);
    initialized_ = true;
}

void GridPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resources, ECS::Registry& registry) {
    static_cast<void>(registry);
    if (!settings_.enabled || (!settings_.showInPlay && ECS::PlayModeController::IsInPlayMode())) return;
    auto* camera = GetTargetCamera();
    if (!camera) return;
    const auto& color = resources.GetColorBuffer("DebugColor");
    if (!initialized_) Initialize();
    const auto& s = settings_;
    const auto& p = camera->forGPU.worldPosition;
    // 描画領域だけをカメラに追従させ、格子自体は基準位置に固定する。
    float u = s.plane == 2 ? p.y : p.x;
    float v = s.plane == 1 ? p.y : p.z;
    GridConstants constants{};
    constants.viewProjection = camera->forGPU.viewProjection;
    std::copy_n(s.origin, 3, constants.originSpacing);
    constants.originSpacing[3] = s.spacing;
    constants.centerRadius[0] = u;
    constants.centerRadius[1] = v;
    constants.centerRadius[2] = s.radius;
    constants.centerRadius[3] = s.fadeStart;
    constants.widthsMajor[0] = s.minorWidth;
    constants.widthsMajor[1] = s.majorWidth;
    constants.widthsMajor[2] = s.axisWidth;
    constants.widthsMajor[3] = static_cast<float>(s.majorEvery);
    constants.options[0] = static_cast<float>(s.plane);
    constants.options[1] = s.adaptive ? 1.f : 0.f;
    constants.options[2] = s.showAxes ? 1.f : 0.f;
    constants.options[3] = s.depthBias;
    std::copy_n(s.minorColor, 4, constants.minorColor);
    std::copy_n(s.majorColor, 4, constants.majorColor);
    constants.cameraPosition[0] = p.x;
    constants.cameraPosition[1] = p.y;
    constants.cameraPosition[2] = p.z;
    auto& indices = RootSignatureBuilder::GetRootIndexMap("Editor Grid PSO");
    gfx.SetViewportAndScissor(0, 0, color.GetWidth(), color.GetHeight());
    gfx.SetRootSignature(rootSignature_);
    gfx.SetPipelineState(s.overlay ? overlayPso_ : pso_);
    gfx.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    gfx.SetDynamicConstantBufferView(indices.at("GridConstants"), sizeof(constants), &constants);
    gfx.Draw(6);
}

void GridPass::DrawEditor() {
#ifdef USE_IMGUI
    auto& state = Editor::EditorWindowManager::GetInstance().Get(Editor::EditorWindowId::Grid);
    if (!state.open) return;
    if (ImGui::Begin(state.label, &state.open)) {
        ImGui::Checkbox("Show grid", &settings_.enabled);
        ImGui::Checkbox("Show while playing", &settings_.showInPlay);
        ImGui::Combo("Plane", &settings_.plane, "XZ\0XY\0YZ\0");
        ImGui::DragFloat3("Origin", settings_.origin, 0.1f);
        ImGui::DragFloat("Cell spacing", &settings_.spacing, 0.05f, 0.01f, 1000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderInt("Major every N cells", &settings_.majorEvery, 2, 10);
        ImGui::Checkbox("Adaptive spacing", &settings_.adaptive);
        ImGui::Checkbox("Show axes", &settings_.showAxes);
        ImGui::Checkbox("Overlay through geometry", &settings_.overlay);
        ImGui::DragFloat("Radius", &settings_.radius, 1.f, 1.f, 10000.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Fade start / radius", &settings_.fadeStart, 0.f, 0.95f);
        ImGui::SliderFloat("Minor width (px)", &settings_.minorWidth, 0.5f, 4.f);
        ImGui::SliderFloat("Major width (px)", &settings_.majorWidth, 0.5f, 4.f);
        ImGui::SliderFloat("Axis width (px)", &settings_.axisWidth, 0.5f, 5.f);
        ImGui::SliderFloat("Depth bias", &settings_.depthBias, 0.f, 0.00001f, "%.7f");
        ImGui::ColorEdit4("Minor color (linear)", settings_.minorColor);
        ImGui::ColorEdit4("Major color (linear)", settings_.majorColor);
        if (ImGui::Button("Reset grid settings")) settings_ = GridSettings{};
        ImGui::TextUnformatted("Display only; snapping is unchanged. Settings are not saved.");
    }
    ImGui::End();
#endif
}
}
