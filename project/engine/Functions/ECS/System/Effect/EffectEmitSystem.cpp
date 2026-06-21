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

		EmitParticle(registry, *emitter, t, e);
		registry.RemoveComponent<EffectEmitTag>(e);
	}
}

void EffectEmitSystem::EmitParticle(Registry& registry, Component::EffectEmitterComponent& effectEmitter, Component::TransformComponent& emitterTransform, Entity entity) {
	for (uint32_t i = 0; i < effectEmitter.count; i++) {
		auto e = registry.GenerateEntity();
		auto* transform = registry.AddComponent<TransformComponent>(e);
		transform->translate = GetNewPosition( effectEmitter, emitterTransform);
		transform->rotation.FromAxisAngle(Math::Vector3::FORWARD, Random::GetRandomVal(effectEmitter.minRotate, effectEmitter.maxRotate));
		transform->scale = Random::GetRandomVal(effectEmitter.minScale, effectEmitter.maxScale);

		if (registry.Has<EffectEmitModeCircleTag>(entity)) {
			
			float phi = Random::GetRandomVal(0.0f, 2.0f * PI); // 周方向角
			// 円盤を一様に埋める（sqrt スケーリング）
			float u = Random::GetRandomVal(0.0f, 1.0f);
			Math::Vector3 speedB = effectEmitter.maxSpeed * std::sqrt(u); // 0..maxSpeed の sqrt スケーリング

			Math::Vector3 dirLocal(std::cos(phi), 0.0f, std::sin(phi)); // XZ 平面の単位方向
			// エミッタの回転を反映する（ローカル→ワールド）
			Math::Vector3 dirWorld = emitterTransform.rotation.RotateVector(dirLocal); // 仮関数名に置換
			dirWorld = dirWorld.Normalize(); // 念のため正規化
			if (registry.GetComponent<EffectEmitModeCircleTag>(entity)->useXYPlane) {
				registry.AddComponent<VelocityComponent>(e)->linear = Math::Vector3(dirWorld.x * speedB.x, dirWorld.z * speedB.z, dirWorld.y * speedB.y);
			} else {
				registry.AddComponent<VelocityComponent>(e)->linear = Math::Vector3(dirWorld.x * speedB.x, dirWorld.y * speedB.y, dirWorld.z * speedB.z);
			}
		} else if (registry.Has<EffectEmitModeRingTag>(entity)) {
			// XZ平面で角度を一様にサンプリング（外周のみ → sqrt不要）
			float phi = Random::GetRandomVal(0.0f, 2.0f * PI);
			Math::Vector3 dirLocal = Math::Vector3(std::cos(phi), 0.0f, std::sin(phi));

			// 速度はmin〜maxの範囲でランダム（sqrtスケーリングなし）
			float speed = Random::GetRandomVal(effectEmitter.minSpeed.x, effectEmitter.maxSpeed.x);

			Math::Vector3 dirWorld = emitterTransform.rotation.RotateVector(dirLocal);
			dirWorld = dirWorld.Normalize();

			registry.AddComponent<VelocityComponent>(e)->linear =
				Math::Vector3(dirWorld.x * speed, dirWorld.y * speed, dirWorld.z * speed);
		} else {
			registry.AddComponent<VelocityComponent>(e)->linear = Random::GetRandomVal(effectEmitter.minSpeed, effectEmitter.maxSpeed);
		}
		


		auto* particle = registry.AddComponent<ParticleComponent>(e);
		particle->lifeTime = Random::GetRandomVal(effectEmitter.minLifeTime, effectEmitter.maxLifeTime);
		particle->texture = effectEmitter.texture;
		particle->color = effectEmitter.color;
		particle->shape = effectEmitter.shape;
	}
}

Math::Vector3 EffectEmitSystem::GetNewPosition(Component::EffectEmitterComponent& effectEmitter, Component::TransformComponent& emitterTransform) {
	Math::Vector3 max = emitterTransform.translate + (effectEmitter.emitRange / 2.0f);
	Math::Vector3 min = emitterTransform.translate - (effectEmitter.emitRange / 2.0f);
	return Random::GetRandomVal(min, max);
}
}
}