#pragma once
#include "RenderPass.h"
#include "engine/Functions/ECS/Component/MeshComponent.h"
#include "engine/Functions/ECS/Component/MaterialComponent.h"
#include "engine/Functions/ECS/Component/TransformComponent.h"
#include "engine/Functions/ECS/Component/CameraComponent.h"

namespace NoEngine {
namespace Render {
class MeshPass :
    public RenderPass {
public:
    MeshPass();
    void Execute(GraphicsContext& gfx, ECS::Registry& registry) override;
private:

    struct DrawItem {
        Component::MeshComponent* mesh;
        Component::MaterialComponent* material;
        Component::TransformComponent* transform;
        uint32_t psoId;
        uint32_t rootSigId;
        std::string psoName;
        float distanceToCamera;
    };

    std::vector<DrawItem> items_;
    Component::CameraComponent* camera_;

    std::string outlinePSOName_;
    std::string outlineSkinnedPSOName_;
    uint32_t outlinePSOID_;
    uint32_t outlineSkinnedPSOID_;

    void Collect(ECS::Registry& registry);
    void Sort();
    void Render(GraphicsContext& gfx);
    void RenderOutline(GraphicsContext& gfx);
};
}
}
