#pragma once
#include "RenderPass.h"
#include "engine/Functions/ECS/Component/MeshComponent.h"
#include "engine/Functions/ECS/Component/MaterialComponent.h"
#include "engine/Functions/ECS/Component/TransformComponent.h"

namespace NoEngine {
namespace Render {

class PrePass :
    public RenderPass {
public:
    PrePass();
    void Execute(GraphicsContext& gfx, ECS::Registry& registry) override;

private:
   /// <summary>
   /// 描画に必要な変数
   /// </summary>
    struct DrawItem {
        Component::MeshComponent* mesh;
        Component::MaterialComponent* material;
        Component::TransformComponent* transform;
    };
    std::vector<DrawItem> items_;
};

}
}
