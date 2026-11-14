#pragma once
#include "../RenderPass.h"

#include "engine/Assets/Model/RaytracingMesh.h"
#include "engine/Math/MathInclude.h"
#include "engine/Functions/ECS/Component/Asset/AnimatorComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"

namespace NoEngine {
namespace Render {
class TLASBuildPass :
    public RenderPass {
public:
    void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
   
private:
    std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instances_;
    Microsoft::WRL::ComPtr<ID3D12Resource> tlasScratch_;
    UINT instanceCapacity_ = 0;
    void BuildRaytracingInstances(ECS::Registry& registry);
    void BuildTLAS(GraphicsContext& gfx);

};
}
}

