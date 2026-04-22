#include "stdafx.h"
#include "BoxColliderUpdateSystem.h"

void BoxColliderUpdateSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto view = registry.View<No::TransformComponent, No::AABBCollider>();
	for (auto e : view) {
		auto* t = registry.GetComponent<No::TransformComponent>(e);
		auto* b = registry.GetComponent<No::AABBCollider>(e);

		b->max = t->scale / 2.f;
		b->min = -t->scale / 2.f;
	}
}
