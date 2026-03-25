#pragma once
#include "engine/NoEngine.h"

class ParticleTestSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	void EmitParticle(No::TransformComponent* transform, No::ParticleEmitterComponent* emitter);
};

