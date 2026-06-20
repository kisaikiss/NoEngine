#include "stdafx.h"
#include "EffectEmitSystem.h"
#include "engine/Functions/ECS/Component/ParticleEmitterComponent.h"
#include "engine/Functions/ECS/Component/VelocityComponent.h"
#include "engine/Assets/AssetManager.h"
#include "engine/Assets/Texture/TextureManager.h"
#include "engine/Utilities/Random.h"

namespace NoEngine {
namespace ECS {
using namespace Component;
void EffectEmitSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto view = registry.View<EffectEmitterComponent, EffectEmitTag, TransformComponent>();
	for (auto e : view) {
		TransformComponent t = *registry.GetComponent<TransformComponent>(e);
		auto* emitter = registry.GetComponent<EffectEmitterComponent>(e);
		if (!emitter->texture.IsValid()) {
			emitter->texture = TextureManager::LoadCovertTexture(AssetManager::GetFilePathFromAddressableName(emitter->textureName));
		}

		EmitParticle(registry, *emitter, t);
		registry.RemoveComponent<EffectEmitTag>(e);
	}
}

void EffectEmitSystem::EmitParticle(Registry& registry, Component::EffectEmitterComponent& effectEmitter, Component::TransformComponent& emitterTransform) {
	for (uint32_t i = 0; i < effectEmitter.count; i++) {
		auto e = registry.GenerateEntity();
		auto* transform = registry.AddComponent<TransformComponent>(e);
		transform->translate = GetNewPosition(effectEmitter, emitterTransform);
		transform->rotation.FromAxisAngle(Math::Vector3::FORWARD, Random::GetRandomVal(effectEmitter.minRotate, effectEmitter.maxRotate));
		transform->scale = Random::GetRandomVal(effectEmitter.minScale, effectEmitter.maxScale);
		registry.AddComponent<VelocityComponent>(e)->linear = Random::GetRandomVal(effectEmitter.minSpeed, effectEmitter.maxSpeed);
		auto* particle = registry.AddComponent<ParticleComponent>(e);
		particle->lifeTime = Random::GetRandomVal(effectEmitter.minLifeTime, effectEmitter.maxLifeTime);
		particle->texture = effectEmitter.texture;
		particle->color = effectEmitter.color;
		particle->shape = effectEmitter.shape;
	}
}

Math::Vector3 EffectEmitSystem::GetNewPosition(Component::EffectEmitterComponent& effectEmitter, Component::TransformComponent& emitterTransform) {
	Math::Vector3 min, max;
	max = emitterTransform.translate + (effectEmitter.emitRange /2.0f);
	min = emitterTransform.translate - (effectEmitter.emitRange / 2.0f);
	Math::Vector3 result;
	result = Random::GetRandomVal(min, max);
	return result;
}
}
}