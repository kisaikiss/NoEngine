#pragma once
#include "../RenderPass.h"
#include "engine/Functions/ECS/Component/Asset/TextComponent.h"
#include "engine/Functions/ECS/Component/Common/Transform2DComponent.h"
#include "engine/Math/Types/Matrix4x4.h"

namespace NoEngine {
namespace Render {

// World／Screen の各空間内で、スプライトとテキストを共通の描画順で処理する。
class SpritePass final : public RenderPass {
public:
    SpritePass();
    ~SpritePass();
    void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
private:
    struct DrawItem {
        Component::Transform2DComponent* transform;
        Component::SpriteComponent* sprite = nullptr;
        Component::TextComponent* text = nullptr;
        int64_t Layer() const { return sprite ? static_cast<int64_t>(sprite->layer) : text->layer; }
        int64_t Order() const { return sprite ? static_cast<int64_t>(sprite->orderInLayer) : text->orderInLayer; }
    };
    std::vector<DrawItem> worldItems_;
    std::vector<DrawItem> screenItems_;
    struct Vertex {
        Math::Vector4 position;
        Math::Vector2 texcoord;
    };
    std::vector<Vertex> vertices_;
    std::vector<uint16_t> indices_;
    struct Batch {
        TextureRef atlasTexture;
        Math::Color color;
        Component::SpriteComponent* sprite = nullptr; // テキストの場合は nullptr
        uint32_t indexStart = 0;
        uint32_t indexCount = 0;
        int32_t baseVertex = 0;
    };
    std::vector<Batch> worldBatches_;
    std::vector<Batch> screenBatches_;
    void CameraUpdate();
    void Collect(ECS::Registry& registry);
    void Sort();
    void MakeLocalQuad(const DrawItem& item, Math::Vector2 out[4]);
    void GenerateVertices(ECS::Registry& registry);
    void AppendSprite(const DrawItem& item, bool isScreenSpace, ECS::Registry& registry, std::vector<Batch>& outBatches);
    void AppendText(const DrawItem& item, bool isScreenSpace, ECS::Registry& registry, std::vector<Batch>& outBatches);
    void Render(GraphicsContext& gfx);
    void DrawBatches(GraphicsContext& gfx, const std::vector<Batch>& batches, const Math::Matrix4x4& cameraMatrix);
};

}
}
