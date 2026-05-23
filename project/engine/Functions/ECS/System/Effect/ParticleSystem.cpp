#include "stdafx.h"
#include "ParticleSystem.h"
#include "engine/Utilities/Random.h"
#include "../../Component/VelocityComponent.h"

namespace NoEngine {
namespace ECS {
using namespace Component;

void ParticleSystem::Update(Registry& registry, float deltaTime) {
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
}
}