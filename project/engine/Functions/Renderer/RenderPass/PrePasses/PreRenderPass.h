#pragma once
#include "../RenderPass.h"
#include "engine/Functions/ECS/Component/Asset/MeshComponent.h"
#include "engine/Functions/ECS/Component/Asset/MaterialComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "engine/Assets/AssetHandles.h"

namespace NoEngine {
namespace Component { struct AnimatorComponent; }
namespace Render {

class PreRenderPass :
    public RenderPass {
public:
    PreRenderPass();
    void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;

};

}
}
