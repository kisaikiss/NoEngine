#pragma once
#include "../ISystem.h"
#include "engine/Functions/ECS/Component/ParticleEmitterComponent.h"
#include "engine/Functions/ECS/Component/TransformComponent.h"
namespace NoEngine {
namespace ECS {
class ParticleEmitterSystem :
    public ISystem {
public:
    void Update(Registry& registry, float deltaTime) override;
private:
    void EmitParticle(Registry& registry, const Component::TransformComponent& transform, Component::ParticleEmitterComponent* emitter);
    Math::Vector3 GetNewPosition(const Component::TransformComponent& emitterTransform);
};

}
}