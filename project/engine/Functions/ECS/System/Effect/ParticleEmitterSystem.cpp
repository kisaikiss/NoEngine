#include "stdafx.h"
#include "ParticleEmitterSystem.h"
#include "engine/Utilities/Random.h"
#include "engine/Assets/AssetManager.h"
#include "../../Component/VelocityComponent.h"

namespace NoEngine {
namespace ECS {
using namespace Component;

void ParticleEmitterSystem::Update(Registry& registry, float deltaTime) {
	auto view = registry.View<ParticleEmitterComponent, TransformComponent>();
	for (auto entity : view) {
		auto* emitter = registry.GetComponent<ParticleEmitterComponent>(entity);
		if (!emitter->active) {
			continue;
		}

		auto* transform = registry.GetComponent<TransformComponent>(entity);
		TransformComponent t = *transform;
		emitter->frequencyTime += deltaTime;
		if (!emitter->texture.IsValid()) {
			emitter->texture = TextureManager::LoadCovertTexture(AssetManager::GetFilePathFromAddressableName(emitter->textureName));
		}

		if (emitter->frequency <= emitter->frequencyTime) {
				EmitParticle(registry, t, emitter);
				emitter->frequencyTime -= emitter->frequency;
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
		particle->color = emitter->color;
		registry.AddComponent<VelocityComponent>(e)->linear = Random::GetRandomVal(emitter->minSpeed, emitter->maxSpeed);
		auto* transform = registry.AddComponent<TransformComponent>(e);
		transform->translate = GetNewPosition(emitterTransform, emitter);
		transform->rotation = Math::Quaternion::IDENTITY; 
		transform->scale = Random::GetRandomVal(emitter->minScale, emitter->maxScale);
	}
}

Math::Vector3 ParticleEmitterSystem::GetNewPosition(const Component::TransformComponent& emitterTransform, Component::ParticleEmitterComponent* emitter) {
	Math::Vector3 min, max;
	max = emitterTransform.translate + (emitter->emitRange / 2.f);
	min = emitterTransform.translate - (emitter->emitRange / 2.f);
	Math::Vector3 result;
	result = Random::GetRandomVal(min, max) + emitter->localPosition;
	return result;
}


}
}