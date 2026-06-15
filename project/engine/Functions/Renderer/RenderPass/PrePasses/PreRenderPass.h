#pragma once
#include "../RenderPass.h"
#include "engine/Functions/ECS/Component/Asset/MeshComponent.h"
#include "engine/Functions/ECS/Component/Asset/MaterialComponent.h"
#include "engine/Functions/ECS/Component/TransformComponent.h"
#include "engine/Assets/AssetHandles.h"

namespace NoEngine {
namespace Render {

class PreRenderPass :
    public RenderPass {
public:
    PreRenderPass();
    void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;

private:
   /// <summary>
   /// 描画に必要な変数
   /// </summary>
    struct DrawItem {
        Asset::MeshHandle mesh;
        Component::MaterialComponent* material;
        Component::TransformComponent* transform;
        Transform* local;
    };
    std::vector<DrawItem> items_;
};

}
}
