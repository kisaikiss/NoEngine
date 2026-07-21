#pragma once
#include "../ISystem.h"
#include "engine/Functions/ECS/Component/Effect/ParticleEmitterComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
namespace NoEngine {
namespace ECS {
class ParticleSystem :
    public ISystem {
public:
    ParticleSystem() { SetStopInPause(false); }
    void Update(Registry& registry, float deltaTime) override;
private:
    void EmitParticle(Registry& registry, const Component::TransformComponent& transform, Component::ParticleEmitterComponent* emitter);
    Math::Vector3 GetNewPosition(const Component::TransformComponent& emitterTransform);
};

}
}

