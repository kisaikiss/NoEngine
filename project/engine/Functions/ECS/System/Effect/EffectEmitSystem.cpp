#include "stdafx.h"
#include "EffectEmitSystem.h"
#include "engine/Functions/ECS/Component/ParticleEmitterComponent.h"
#include "engine/Functions/ECS/Component/VelocityComponent.h"
#include "engine/Utilities/Random.h"
namespace NoEngine {
namespace ECS {
using namespace Component;
void EffectEmitSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto view = registry.View<EffectEmitterComponent, EffectEmitTag, TransformComponent>();
	for (auto e : view) {
		TransformComponent t = *registry.GetComponent<TransformComponent>(e);
		EmitParticle(registry, *registry.GetComponent<EffectEmitterComponent>(e), t);
		registry.RemoveComponent<EffectEmitTag>(e);
	}
}
void EffectEmitSystem::EmitParticle(Registry& registry, Component::EffectEmitterComponent& effectEmitter, Component::TransformComponent& emitterTransform) {
	for (uint32_t i = 0; i < effectEmitter.count; i++) {
		auto e = registry.GenerateEntity();
		auto* transform = registry.AddComponent<TransformComponent>(e);
		transform->translate = emitterTransform.translate;
		transform->rotation.FromAxisAngle(Math::Vector3::FORWARD, Random::GetRandomVal(effectEmitter.minRotate, effectEmitter.maxRotate));
		transform->scale = Random::GetRandomVal(effectEmitter.minScale, effectEmitter.maxScale);
		registry.AddComponent<VelocityComponent>(e)->linear = Random::GetRandomVal(effectEmitter.minSpeed, effectEmitter.maxSpeed);
		auto* particle = registry.AddComponent<ParticleComponent>(e);
		particle->lifeTime = Random::GetRandomVal(effectEmitter.minLifeTime, effectEmitter.maxLifeTime);
		particle->texture = effectEmitter.texture;
		particle->shape = effectEmitter.shape;
	}
}
}
}