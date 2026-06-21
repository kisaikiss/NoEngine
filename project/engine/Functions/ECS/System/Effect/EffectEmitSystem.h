#pragma once
#include "../ISystem.h"
#include "engine/Functions/ECS/Component/Effect/EffectEmitterComponent.h"
#include "engine/Functions/ECS/Component/TransformComponent.h"
namespace NoEngine {
namespace ECS {

class EffectEmitSystem :
    public ISystem {
public:
    EffectEmitSystem() { SetStopInGameStop(false); SetStopInPause(false); }
    void Update(Registry& registry, float deltaTime) override;
private:
    void EmitParticle(Registry& registry, Component::EffectEmitterComponent& effectEmitter, Component::TransformComponent& emitterTransform, Entity entity);
    Math::Vector3 GetNewPosition(Component::EffectEmitterComponent& effectEmitter, Component::TransformComponent& emitterTransform);
};

}
}