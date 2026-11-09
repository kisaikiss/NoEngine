#include "stdafx.h"
#include "EffectEmitSystem.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "engine/Functions/ECS/Component/Common/VelocityComponent.h"
#include "engine/Assets/AssetManager.h"
#include "engine/Utilities/Random.h"

namespace NoEngine {
namespace ECS {
using namespace Component;

namespace {

Math::Vector3 SampleCircleXZ() {
	const float angle = Random::GetRandomVal(0.0f, 2.0f * PI);
	return { std::cos(angle), 0.0f, std::sin(angle) };
}

Math::Vector3 SampleSphereDirection() {
	const float y = Random::GetRandomVal(-1.0f, 1.0f);
	const float xz = std::sqrt(1.0f - y * y);
	const auto circle = SampleCircleXZ();
	return { circle.x * xz, y, circle.z * xz };
}

Math::Vector3 SampleSpawnOffset(const EffectPreset& preset) {
	const float radius = std::max(0.0f, preset.spawnRadius);
	switch (preset.spawnShape) {
	case EffectSpawnShape::kPoint:
		return Math::Vector3::ZERO;
	case EffectSpawnShape::kDiskXZ:
		return SampleCircleXZ() * (radius * std::sqrt(Random::GetRandomVal(0.0f, 1.0f)));
	case EffectSpawnShape::kRingXZ:
		return SampleCircleXZ() * radius;
	case EffectSpawnShape::kSphere:
		return SampleSphereDirection() * (radius * std::cbrt(Random::GetRandomVal(0.0f, 1.0f)));
	case EffectSpawnShape::kBox:
	default: {
		const auto extent = preset.emitRange / 2.0f;
		return Random::GetRandomVal(-extent, extent);
	}
	}
}

Math::Vector3 SampleVelocity(const EffectPreset& preset, const Math::Quaternion& rotation) {
	switch (preset.velocityDistribution) {
	case EffectVelocityDistribution::kDiskXZ:
	case EffectVelocityDistribution::kDiskXY: {
		const auto direction = rotation.RotateVector(SampleCircleXZ()).Normalize();
		const auto speed = preset.maxSpeed * std::sqrt(Random::GetRandomVal(0.0f, 1.0f));
		const Math::Vector3 velocity{ direction.x * speed.x, direction.y * speed.y, direction.z * speed.z };
		if (preset.velocityDistribution == EffectVelocityDistribution::kDiskXY) {
			return { velocity.x, velocity.z, velocity.y };
		}
		return velocity;
	}
	case EffectVelocityDistribution::kRingXZ:
	case EffectVelocityDistribution::kSphere: {
		const auto direction = preset.velocityDistribution == EffectVelocityDistribution::kRingXZ
			? SampleCircleXZ() : SampleSphereDirection();
		const float speed = Random::GetRandomVal(preset.minSpeed.x, preset.maxSpeed.x);
		return rotation.RotateVector(direction).Normalize() * speed;
	}
	case EffectVelocityDistribution::kBox:
	default:
		return Random::GetRandomVal(preset.minSpeed, preset.maxSpeed);
	}
}


void EmitBurst(Registry& registry, const EffectPreset& preset, const Math::Vector3& worldPosition, const Math::Quaternion& rotation) {
	const auto center = worldPosition + preset.spawnOffset;
	for (uint32_t i = 0; i < preset.count; ++i) {
		const auto entity = registry.GenerateEntity();
		auto* transform = registry.AddComponent<TransformComponent>(entity);
		transform->translate = center + SampleSpawnOffset(preset);
		transform->rotation.FromAxisAngle(Math::Vector3::FORWARD, Random::GetRandomVal(preset.minRotate, preset.maxRotate));
		transform->scale = Random::GetRandomVal(preset.minScale, preset.maxScale);
		registry.AddComponent<VelocityComponent>(entity)->linear = SampleVelocity(preset, rotation);

		auto* particle = registry.AddComponent<ParticleComponent>(entity);
		particle->lifeTime = Random::GetRandomVal(preset.minLifeTime, preset.maxLifeTime);
		particle->texture = preset.texture;
		particle->color = preset.color;
		particle->shape = preset.shape;
	}
}

} // namespace

void Registry::PlayEffect(const EffectPreset& preset, const Math::Vector3& worldPosition) {
	EmitEvent(EffectEmitSystem::Playback{ preset, worldPosition });
}

void Registry::PlayEffect(Entity presetEntity, const Math::Vector3& worldPosition) {
	const auto* emitter = GetComponent<EffectEmitterComponent>(presetEntity);
	if (!emitter) return;

	auto preset = *emitter;
	const auto* transform = GetComponent<TransformComponent>(presetEntity);
	const auto rotation = transform ? transform->rotation : Math::Quaternion::IDENTITY;
	EmitEvent(EffectEmitSystem::Playback{ std::move(preset), worldPosition, rotation });
}

void EffectEmitSystem::Update(Registry& registry, float deltaTime) {
	for (auto entity : registry.View<EffectEmitterComponent, EffectEmitTag, TransformComponent>()) {
		const auto* transform = registry.GetComponent<TransformComponent>(entity);
		registry.PlayEffect(entity, transform->GetWorldPosition(registry));
		registry.RemoveComponent<EffectEmitTag>(entity);
	}

	for (auto& request : registry.PollAllEvents<Playback>()) {
		auto& preset = request.preset;
		if (preset.count == 0 || preset.burstCount == 0) continue;

		if (!preset.texture.IsValid() && !preset.textureName.empty()) {
			const auto path = AssetManager::GetFilePathFromAddressableName(preset.textureName);
			if (!path.empty()) preset.texture = TextureManager::LoadCovertTexture(path);
		}
		request.timeToNextBurst = std::isfinite(preset.startDelay) ? std::max(0.0f, preset.startDelay) : 0.0f;
		request.burstsRemaining = std::isfinite(preset.burstInterval) && preset.burstInterval > 0.0f
			? preset.burstCount : 1;
		playbacks_.push_back(std::move(request));
	}

	const double elapsed = std::isfinite(deltaTime) ? std::max(0.0f, deltaTime) : 0.0f;
	for (auto& playback : playbacks_) {
		playback.timeToNextBurst -= elapsed;
		// 遅れた発生は繰り越し、1 回の更新で発生が集中しすぎないようにする。
		constexpr int kMaxBurstsPerUpdate = 32;
		for (int i = 0; i < kMaxBurstsPerUpdate && playback.burstsRemaining > 0 && playback.timeToNextBurst <= 0.0; ++i) {
			EmitBurst(registry, playback.preset, playback.worldPosition, playback.rotation);
			--playback.burstsRemaining;
			playback.timeToNextBurst += playback.preset.burstInterval;
		}
	}
	std::erase_if(playbacks_, [](const Playback& playback) { return playback.burstsRemaining == 0; });
}

}
}
#include "../SystemManager.h"
REGISTER_SYSTEM(NoEngine::ECS::EffectEmitSystem, "EffectEmitSystem", "Effects")
