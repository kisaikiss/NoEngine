#pragma once
#include "../RenderPass.h"
#include "engine/Functions/ECS/Component/Asset/SpriteComponent.h"
#include "engine/Functions/ECS/Component/Common/Transform2DComponent.h"
#include "engine/Math/Types/Matrix4x4.h"

namespace NoEngine {
namespace Render {

// SpritePassと同じ描画順序(layer/orderInLayer)・同じ空間変換でスプライトのクアッドを描画し、
// 色の代わりにEntity IDをObjectIDバッファへ書き込むパス。
// シーンビューのクリックでスプライトを選択できるようにするため、PreRenderPass(メッシュ用)と
// 同じ仕組みでIDを持たせる。深度テストなし・ブレンドなしでSpritePassと同じ「後勝ち」順にする。
class SpriteIDPass final :
    public RenderPass {
public:
    void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
private:
    struct DrawItem {
        Component::Transform2DComponent* transform;
        Component::SpriteComponent* sprite;
        ECS::Entity entity;
    };
    std::vector<DrawItem> items_;

    struct SpriteVertex {
        Math::Vector4 position;
        Math::Vector2 texcoord;
    };

    void Collect(ECS::Registry& registry);
    void Sort();
    void MakeLocalQuad(const DrawItem& item, Math::Vector2 out[4]);
};

}
}