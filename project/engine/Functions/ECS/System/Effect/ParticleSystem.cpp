#include "stdafx.h"
#include "ParticleSystem.h"
#include "engine/Utilities/Random.h"
#include "../../Component/VelocityComponent.h"

namespace NoEngine {
namespace ECS {
using namespace Component;

void ParticleSystem::Update(Registry& registry, float deltaTime) {
	auto view = registry.View<ParticleEmitterComponent, TransformComponent>();
	for (auto entity : view) {
		auto* emitter = registry.GetComponent<ParticleEmitterComponent>(entity);
		auto* transform = registry.GetComponent<TransformComponent>(entity);
		TransformComponent t = *transform;
		emitter->frequencyTime += deltaTime;
		if (emitter->frequency <= emitter->frequencyTime) {
			EmitParticle(registry, t, emitter);
			emitter->frequencyTime -= emitter->frequency;
		}

	}

	auto particleView = registry.View<ParticleComponent, TransformComponent, VelocityComponent>();
	for (auto entity : particleView) {
		auto* particle = registry.GetComponent<ParticleComponent>(entity);
		particle->currentTime += deltaTime;
		particle->color.a = 1.0f - (particle->currentTime / particle->lifeTime);
		if (particle->currentTime >= particle->lifeTime) {		
			registry.DestroyEntity(entity);
		}
	}
}

void ParticleSystem::EmitParticle(Registry& registry, const Component::TransformComponent& emitterTransform, Component::ParticleEmitterComponent* emitter) {
	for (uint32_t count = 0; count < emitter->count; count++) {
		auto e = registry.GenerateEntity();
		auto* particle = registry.AddComponent<ParticleComponent>(e);
		particle->lifeTime = Random::GetRandomVal(emitter->minLifeTime, emitter->maxLifeTime);
		particle->texture = emitter->texture;
		registry.AddComponent<VelocityComponent>(e)->linear = Random::GetRandomVal(emitter->minSpeed, emitter->maxSpeed);
		auto* transform = registry.AddComponent<TransformComponent>(e);
		transform->translate = GetNewPosition(emitterTransform);
		transform->rotation = Math::Quaternion::IDENTITY;
		transform->scale = Math::Vector3::UNIT_SCALE;
	}
}

Math::Vector3 ParticleSystem::GetNewPosition(const Component::TransformComponent& emitterTransform) {
	Math::Vector3 min, max;
	max = emitterTransform.translate + emitterTransform.scale;
	min = emitterTransform.translate - emitterTransform.scale;
	Math::Vector3 result;
	result = Random::GetRandomVal(min, max);
	return result;
}


}
}