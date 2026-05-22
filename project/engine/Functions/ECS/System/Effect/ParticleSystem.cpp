#include "stdafx.h"
#include "ParticleSystem.h"
#include "engine/Utilities/Random.h"

namespace NoEngine {
namespace ECS {
using namespace Component;

void ParticleSystem::Update(Registry& registry, float deltaTime) {
	auto view = registry.View<ParticleEmitterComponent>();
	for (auto entity : view) {
		auto* emitter = registry.GetComponent<ParticleEmitterComponent>(entity);
		auto* transform = registry.GetComponent<TransformComponent>(entity);
		for (auto& p : emitter->particles) {
			p.transform.translate += p.velocity * deltaTime;
			p.currentTime += deltaTime;
			p.color.a = 1.0f - (p.currentTime / p.lifeTime);
		}

		for (uint32_t i = 0; i < emitter->particles.size(); i++) {
			if (emitter->particles[i].currentTime >= emitter->particles[i].lifeTime) {
				emitter->particles.erase(emitter->particles.begin() + i);
			}
		};

		emitter->frequencyTime += deltaTime;
		if (emitter->frequency <= emitter->frequencyTime) {
			EmitParticle(transform, emitter);
			emitter->frequencyTime -= emitter->frequency;
		}

	}
}

void ParticleSystem::EmitParticle(Component::TransformComponent* transform, Component::ParticleEmitterComponent* emitter) {
	for (uint32_t count = 0; count < emitter->count; count++) {
		Particle particle{};
		particle.transform = Transform(transform->translate, Math::Quaternion::IDENTITY, Math::Vector3::UNIT_SCALE);
		particle.velocity = Random::GetRandomVal(emitter->minSpeed, emitter->maxSpeed);
		particle.lifeTime = Random::GetRandomVal(emitter->minLifeTime, emitter->maxLifeTime);
		emitter->particles.push_back(particle);
	}
}


}
}