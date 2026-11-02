#include "SpritePass.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#include "engine/Math/Types/Calculations/Matrix4x4Calculations.h"
#include "engine/Functions/ECS/Component/Common/CameraComponent.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Utilities/Conversion/ConvertString.h"


namespace NoEngine {
namespace Render {

using namespace Component;
using namespace Math;
namespace {
Matrix4x4 sCameraMatrix;
}

NoEngine::Render::SpritePass::SpritePass() {

    auto size = GraphicsCore::sWindowManager.GetMainWindow()->GetWindowSize();
    sCameraMatrix = MathCalculations::MakeOrthographicMatrix(0.f, 0.f, static_cast<float>(size.clientWidth), static_cast<float>(size.clientHeight), 0.1f, 100.f);

}

SpritePass::~SpritePass() {}

void SpritePass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
    (void)resourceRegistry;
    CameraUpdate();
    Collect(registry);
    Sort();
    GenerateVertices(registry);
    Render(gfx);
}

void SpritePass::CameraUpdate() {
    auto* camera = GetTargetCamera2D();
    if (camera) {
        sCameraMatrix = camera->viewProjection;
    } else {
        Vector2 windowSize = GraphicsCore::GetWindowSize();
        sCameraMatrix = MathCalculations::MakeOrthographicMatrix(0.f, 0.f, static_cast<float>(windowSize.x), static_cast<float>(windowSize.y), 0.1f, 100.f);
    }

}

void SpritePass::Collect(ECS::Registry& registry) {
    worldItems_.clear();
    screenItems_.clear();
    for (auto entity : registry.View<Transform2DComponent, SpriteComponent>()) {
        auto* sprite = registry.GetComponent<SpriteComponent>(entity);
        if (!sprite->isVisible || !sprite->textureHandle.IsValid()) continue;
        auto* transform = registry.GetComponent<Transform2DComponent>(entity);
        auto& items = sprite->space == SpriteSpace::Screen ? screenItems_ : worldItems_;
        items.push_back({ transform, sprite, nullptr });
    }
    for (auto entity : registry.View<Transform2DComponent, TextComponent>()) {
        auto* text = registry.GetComponent<TextComponent>(entity);
        if (!text->isVisible || text->text.empty() || !text->fontHandle.IsValid()) continue;
        auto* transform = registry.GetComponent<Transform2DComponent>(entity);
        auto& items = text->space == SpriteSpace::Screen ? screenItems_ : worldItems_;
        items.push_back({ transform, nullptr, text });
    }
}

void SpritePass::Sort() {
    auto comparator = [](const DrawItem& a, const DrawItem& b) {
        if (a.Layer() != b.Layer()) return a.Layer() < b.Layer();
        if (a.Order() != b.Order()) return a.Order() < b.Order();
        // レイヤーと順序が同じ場合は、スプライトの後にテキストを描画する。
        if (!!a.sprite != !!b.sprite) return a.sprite != nullptr;
        if (a.sprite) {
            if (a.sprite->textureHandle != b.sprite->textureHandle) return a.sprite->textureHandle < b.sprite->textureHandle;
            return a.sprite->color < b.sprite->color;
        }
        return false;
    };
    std::stable_sort(worldItems_.begin(), worldItems_.end(), comparator);
    std::stable_sort(screenItems_.begin(), screenItems_.end(), comparator);
}

void SpritePass::MakeLocalQuad(const DrawItem& item, Vector2 out[4]) {
    Vector2 size = { 1.f,1.f };

    // pivot を中心にローカル座標を作る
    Vector2 origin{ size.x * item.sprite->pivot.x, size.y * item.sprite->pivot.y };

    float left = -origin.x;
    float right = size.x - origin.x;
    float top = -origin.y;
    float bottom = size.y - origin.y;

    if (item.sprite->flipX) std::swap(left, right);
    if (item.sprite->flipY) std::swap(top, bottom);

    out[0] = { left,  top };
    out[1] = { right, top };
    out[2] = { left,  bottom };
    out[3] = { right, bottom };
}

void SpritePass::AppendSprite(const DrawItem& item, bool isScreenSpace, ECS::Registry& registry, std::vector<Batch>& outBatches) {
    Batch batch;
    batch.sprite = item.sprite;
    batch.atlasTexture = item.sprite->textureHandle;
    batch.color = item.sprite->color;
    batch.indexStart = static_cast<uint32_t>(indices_.size());
    batch.baseVertex = static_cast<int32_t>(vertices_.size());
    batch.indexCount = 6;
    Vector2 local[4];
    MakeLocalQuad(item, local);
    Matrix4x4 mat = item.transform->MakeAffineMatrix4x4(registry);
    Vector2 windowSize = GraphicsCore::GetWindowSize();
    Rect uv = item.sprite->uv;
    Vector2 uvs[4] = {
        { uv.x, uv.y }, { uv.x + uv.width, uv.y },
        { uv.x, uv.y + uv.height }, { uv.x + uv.width, uv.y + uv.height }
    };
    for (int i = 0; i < 4; ++i) {
        Vector3 world = MathCalculations::Transform(Vector3(local[i].x, local[i].y, 0.f), mat);
        if (isScreenSpace) {
            world.x += item.sprite->anchor.x * windowSize.x;
            world.y += item.sprite->anchor.y * windowSize.y;
        }
        vertices_.push_back({ { world.x, world.y, 0.f, 1.f }, uvs[i] });
    }
    // 描画順で隣接し、マテリアルとマスク設定が同一のスプライトだけをまとめて描画する。
    Batch* previous = outBatches.empty() ? nullptr : &outBatches.back();
    const auto* a = previous ? previous->sprite : nullptr;
    const auto* b = item.sprite;
    const bool merge = a && previous->atlasTexture == batch.atlasTexture &&
        previous->color.r == batch.color.r && previous->color.g == batch.color.g &&
        previous->color.b == batch.color.b && previous->color.a == batch.color.a &&
        a->useMask == b->useMask && a->maskTextureHandle == b->maskTextureHandle &&
        a->fill == b->fill && a->nonRenderAngle == b->nonRenderAngle &&
        vertices_.size() - previous->baseVertex <= 65536;
    const uint16_t offset = merge ? static_cast<uint16_t>(batch.baseVertex - previous->baseVertex) : 0;
    for (uint16_t index : std::initializer_list<uint16_t>{ 0, 1, 2, 1, 3, 2 }) indices_.push_back(offset + index);
    if (merge) previous->indexCount += batch.indexCount;
    else outBatches.push_back(batch);
}

void SpritePass::AppendText(const DrawItem& item, bool isScreenSpace, ECS::Registry& registry, std::vector<Batch>& outBatches) {
    auto* t = item.transform;
    auto* text = item.text;

    Font* font = text->fontHandle.Get();
    if (!font) return;

    std::u32string codepoints = Utf8ToCodepoints(text->text);

    // '\n'で行分割する(hAlignの計算に各行の幅が必要なため)
    std::vector<std::u32string> lines;
    {
        std::u32string current;
        for (char32_t c : codepoints) {
            if (c == U'\n') { lines.push_back(current); current.clear(); } else current.push_back(c);
        }
        lines.push_back(current);
    }

    std::vector<float> lineWidths;
    lineWidths.reserve(lines.size());
    for (auto& line : lines) {
        float width = 0.f;
        for (char32_t c : line) {
            const GlyphInfo* g = font->GetGlyph(c);
            if (!g) continue;
            width += g->advance + text->letterSpacing;
        }
        lineWidths.push_back(width);
    }

    const float ascent = font->GetAscent();
    const float lineHeight = font->GetLineHeight();
    const float blockHeight = lineHeight * static_cast<float>(lines.size());

    float vOffset = 0.f;
    switch (text->vAlign) {
    case TextComponent::VAlign::kTop:    vOffset = ascent; break;
    case TextComponent::VAlign::kMiddle: vOffset = ascent - blockHeight * 0.5f; break;
    case TextComponent::VAlign::kBaseline:
    default: vOffset = 0.f; break;
    }

    Matrix4x4 mat = t->MakeAffineMatrix4x4(registry);
    Vector2 windowSize = GraphicsCore::GetWindowSize();

    Batch batch;
    batch.atlasTexture = font->GetAtlasTexture();
    batch.color = text->color;
    batch.indexStart = static_cast<uint32_t>(indices_.size());
    batch.baseVertex = static_cast<int32_t>(vertices_.size());

    for (size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
        const std::u32string& line = lines[lineIndex];

        float hOffset = 0.f;
        switch (text->hAlign) {
        case TextComponent::HAlign::kCenter: hOffset = -lineWidths[lineIndex] * 0.5f; break;
        case TextComponent::HAlign::kRight:  hOffset = -lineWidths[lineIndex]; break;
        case TextComponent::HAlign::kLeft:
        default: hOffset = 0.f; break;
        }

        float cursorX = hOffset;
        const float cursorY = vOffset + lineHeight * static_cast<float>(lineIndex);

        for (char32_t c : line) {
            const GlyphInfo* g = font->GetGlyph(c);
            if (!g) continue;

            if (g->size.x > 0.f && g->size.y > 0.f) {
                const float localLeft = (cursorX + g->bearing.x) * text->scale;
                const float localTop = (cursorY + g->bearing.y) * text->scale;
                const float localRight = localLeft + g->size.x * text->scale;
                const float localBottom = localTop + g->size.y * text->scale;

                Vector3 local[4] = {
                    { localLeft,  localTop,    0.f },
                    { localRight, localTop,    0.f },
                    { localLeft,  localBottom, 0.f },
                    { localRight, localBottom, 0.f },
                };

                Vector2 uvs[4] = {
                    { g->uv.x,               g->uv.y },
                    { g->uv.x + g->uv.width, g->uv.y },
                    { g->uv.x,               g->uv.y + g->uv.height },
                    { g->uv.x + g->uv.width, g->uv.y + g->uv.height },
                };

                if (vertices_.size() - batch.baseVertex + 4 > 65536) {
                    batch.indexCount = static_cast<uint32_t>(indices_.size()) - batch.indexStart;
                    outBatches.push_back(batch);
                    batch.indexStart = static_cast<uint32_t>(indices_.size());
                    batch.baseVertex = static_cast<int32_t>(vertices_.size());
                }
                uint16_t indexOffset = static_cast<uint16_t>(vertices_.size() - batch.baseVertex);
                for (int i = 0; i < 4; ++i) {
                    Vector3 world = MathCalculations::Transform(local[i], mat);
                    if (isScreenSpace) {
                        world.x += text->anchor.x * windowSize.x;
                        world.y += text->anchor.y * windowSize.y;
                    }
                    vertices_.push_back({ { world.x, world.y, 0.f, 1.f }, uvs[i] });
                }

                indices_.push_back(indexOffset + 0);
                indices_.push_back(indexOffset + 1);
                indices_.push_back(indexOffset + 2);
                indices_.push_back(indexOffset + 1);
                indices_.push_back(indexOffset + 3);
                indices_.push_back(indexOffset + 2);
            }

            cursorX += g->advance + text->letterSpacing;
        }
    }

    batch.indexCount = static_cast<uint32_t>(indices_.size()) - batch.indexStart;
    if (batch.indexCount > 0) {
        outBatches.push_back(batch);
    }
}

void SpritePass::GenerateVertices(ECS::Registry& registry) {
    vertices_.clear();
    indices_.clear();
    worldBatches_.clear();
    screenBatches_.clear();
    // 後続のテキストでアトラスのサイズが変わる可能性があるため、ハンドルと UV の取得前に全グリフをベイクする。
    auto bake = [](const std::vector<DrawItem>& items) {
        for (const auto& item : items) {
            if (item.text) {
                if (auto* font = item.text->fontHandle.Get()) font->EnsureGlyphsBaked(Utf8ToCodepoints(item.text->text));
            }
        }
    };
    bake(worldItems_);
    bake(screenItems_);
    auto append = [&](const std::vector<DrawItem>& items, bool screen, std::vector<Batch>& batches) {
        for (const auto& item : items) {
            if (item.sprite) AppendSprite(item, screen, registry, batches);
            else AppendText(item, screen, registry, batches);
        }
    };
    append(worldItems_, false, worldBatches_);
    append(screenItems_, true, screenBatches_);
}

void SpritePass::Render(GraphicsContext& gfx) {
    if (vertices_.empty()) return;
    gfx.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    gfx.SetDynamicVB(0, vertices_.size(), sizeof(Vertex), vertices_.data());
    gfx.SetDynamicIB(indices_.size(), indices_.data());
    DrawBatches(gfx, worldBatches_, sCameraMatrix);
    Vector2 size = GraphicsCore::GetWindowSize();
    Matrix4x4 screenMatrix = MathCalculations::MakeOrthographicMatrix(0.f, 0.f, size.x, size.y, 0.1f, 100.f);
    DrawBatches(gfx, screenBatches_, screenMatrix);
}

void SpritePass::DrawBatches(GraphicsContext& gfx, const std::vector<Batch>& batches, const Matrix4x4& cameraMatrix) {
    auto* renderCtx = GetRenderContext();
    const char* currentPSO = nullptr;
    for (const auto& batch : batches) {
        const char* pso = batch.sprite ? "Renderer : Default Sprite PSO" : "Renderer : Text PSO";
        auto& rootIndex = RootSignatureBuilder::GetRootIndexMap(pso);
        if (currentPSO != pso) {
            gfx.SetPipelineState(renderCtx->GetGraphicsPSO(pso));
            gfx.SetRootSignature(renderCtx->GetRootSignature(pso));
            gfx.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(Matrix4x4), &cameraMatrix);
            currentPSO = pso;
        }
        alignas(16) Color material = batch.color;
        gfx.SetDynamicConstantBufferView(rootIndex["gMaterial"], sizeof(material), &material);
        if (batch.sprite) {
            const auto* sprite = batch.sprite;
            struct alignas(16) MaskConstants {
                float fill;
                int useMask;
                float angle;
                float pad;
            } mask{ sprite->fill, sprite->useMask && sprite->maskTextureHandle.IsValid() ? 1 : 0, sprite->nonRenderAngle, 0.f };
            gfx.SetDynamicConstantBufferView(rootIndex["gMaskParams"], sizeof(mask), &mask);
            const auto& maskTexture = sprite->maskTextureHandle.IsValid() ? sprite->maskTextureHandle : batch.atlasTexture;
            gfx.SetDynamicDescriptor(rootIndex["gMask"], 0, maskTexture.GetSRV());
        }
        gfx.SetDynamicDescriptor(rootIndex["gTexture"], 0, batch.atlasTexture.GetSRV());
        gfx.DrawIndexedInstanced(batch.indexCount, 1, batch.indexStart, batch.baseVertex, 0);
    }
}

}
}
