#pragma once
#include "../ISystem.h"
#include "engine/Functions/ECS/Component/ParticleEmitterComponent.h"
#include "engine/Functions/ECS/Component/TransformComponent.h"

#include "engine/Runtime/PipelineStateObject/RootSignature.h"
#include "engine/Runtime/PipelineStateObject/ComputePSO.h"
namespace NoEngine {
namespace ECS {
class ParticleEmitterSystem :
    public ISystem {
public:
    ParticleEmitterSystem();
    void Update(ComputeContext& ctx, Registry& registry, float deltaTime) override;
    void Update(Registry& registry, float deltaTime) override;
private:
    void EmitParticle(Registry& registry, const Component::TransformComponent& transform, Component::ParticleEmitterComponent* emitter);
    Math::Vector3 GetNewPosition(const Component::TransformComponent& emitterTransform, Component::ParticleEmitterComponent* emitter);

    ComputePSO pso_;
    RootSignature rootSignature_;
};

}
}