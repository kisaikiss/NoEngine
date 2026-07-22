#pragma once
#include "../ISystem.h"
#include "engine/Functions/ECS/Component/Effect/ParticleEmitterComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"

#include "engine/Runtime/PipelineStateObject/RootSignature.h"
#include "engine/Runtime/PipelineStateObject/ComputePSO.h"
namespace NoEngine {
namespace ECS {
class ParticleSystem :
    public ISystem {
public:
    ParticleSystem();
    void Update(ComputeContext& ctx, Registry& registry, float deltaTime) override;
    void Update(Registry& registry, float deltaTime) override;
private:

    ComputePSO pso_;
    RootSignature rootSignature_;
};

}
}

