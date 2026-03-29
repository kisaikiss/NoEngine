#include "stdafx.h"
#include "LifetimeSystem.h"
#include "application/CommentBout/Component/LifetimeComponent.h"

void LifetimeSystem::Update(No::Registry& registry, float deltaTime)
{
	auto view = registry.View<LifetimeComponent>();
	for (auto entity : view) {
		auto* lifetime = registry.GetComponent<LifetimeComponent>(entity);
		if (!lifetime) {
			continue;
		}

		lifetime->remainingTime -= deltaTime;
		if (lifetime->remainingTime <= 0.0f) {
			registry.DestroyEntity(entity);
		}
	}
}
