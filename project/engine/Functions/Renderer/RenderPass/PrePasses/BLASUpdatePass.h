#pragma once
#include "../RenderPass.h"

#include "engine/Assets/Model/RaytracingMesh.h"
#include "engine/Math/MathInclude.h"
#include "engine/Functions/ECS/Component/Asset/AnimatorComponent.h"

namespace NoEngine {
namespace Render {
class BLASUpdatePass :
    public RenderPass {
public:
    void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
};
}
}