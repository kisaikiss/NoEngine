#include "stdafx.h"
#include "ParticleEmitterSystem.h"
#include "engine/Utilities/Random.h"
#include "../../Component/VelocityComponent.h"

namespace NoEngine {
namespace ECS {
using namespace Component;

void ParticleEmitterSystem::Update(Registry& registry, float deltaTime) {
	auto view = registry.View<ParticleEmitterComponent, TransformComponent>();
	for (auto entity : view) {
		auto* emitter = registry.GetComponent<ParticleEmitterComponent>(entity);
		auto* transform = registry.GetComponent<TransformComponent>(entity);
		TransformComponent t = *transform;
		emitter->frequencyTime += deltaTime;
		if (emitter->frequency <= emitter->frequencyTime) {
			if (emitter->active) {
				EmitParticle(registry, t, emitter);
				emitter->frequencyTime -= emitter->frequency;
			}
		}
	}
}

void ParticleEmitterSystem::EmitParticle(Registry& registry, const Component::TransformComponent& emitterTransform, Component::ParticleEmitterComponent* emitter) {
	for (uint32_t count = 0; count < emitter->count; count++) {
		auto e = registry.GenerateEntity();
		auto* particle = registry.AddComponent<ParticleComponent>(e);
		particle->lifeTime = Random::GetRandomVal(emitter->minLifeTime, emitter->maxLifeTime);
		particle->texture = emitter->texture;
		particle->shape = emitter->shape;
		registry.AddComponent<VelocityComponent>(e)->linear = Random::GetRandomVal(emitter->minSpeed, emitter->maxSpeed);
		auto* transform = registry.AddComponent<TransformComponent>(e);
		transform->translate = GetNewPosition(emitterTransform);
		transform->rotation = Math::Quaternion::IDENTITY;
		transform->scale = Math::Vector3::UNIT_SCALE;
	}
}

Math::Vector3 ParticleEmitterSystem::GetNewPosition(const Component::TransformComponent& emitterTransform) {
	Math::Vector3 min, max;
	max = emitterTransform.translate + emitterTransform.scale;
	min = emitterTransform.translate - emitterTransform.scale;
	Math::Vector3 result;
	result = Random::GetRandomVal(min, max);
	return result;
}


}
}