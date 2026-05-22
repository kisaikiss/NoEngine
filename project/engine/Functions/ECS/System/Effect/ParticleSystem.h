#pragma once
#include "../ISystem.h"
#include "engine/Functions/ECS/Component/ParticleEmitterComponent.h"
#include "engine/Functions/ECS/Component/TransformComponent.h"
namespace NoEngine {
namespace ECS {
class ParticleSystem :
    public ISystem {
public:
    ParticleSystem() { SetStopInPause(false); }
    void Update(Registry& registry, float deltaTime) override;
private:
    void EmitParticle(Component::TransformComponent* transform, Component::ParticleEmitterComponent* emitter);
};

}
}

