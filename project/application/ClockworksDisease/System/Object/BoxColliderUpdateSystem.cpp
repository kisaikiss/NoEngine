#include "stdafx.h"
#include "BoxColliderUpdateSystem.h"

#include "../../Component/Stage/StageComponent.h"

void BoxColliderUpdateSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	for (auto e : registry.View<No::TransformComponent, No::AABBCollider, ColliderUpdateTag>()) {
		auto* t = registry.GetComponent<No::TransformComponent>(e);
		auto* b = registry.GetComponent<No::AABBCollider>(e);

		b->max = t->scale / 2.f;
		b->min = -t->scale / 2.f;
		if (b->max.x < b->min.x) {
			float tmp = b->max.x;
			b->max.x = b->min.x;
			b->min.x = tmp;
		}

		if (b->max.y < b->min.y) {
			float tmp = b->max.y;
			b->max.y = b->min.y;
			b->min.y = tmp;
		}

		if (b->max.z < b->min.z) {
			float tmp = b->max.z;
			b->max.z = b->min.z;
			b->min.z = tmp;
		}
	}

	for (auto e : registry.View<No::TransformComponent, No::OBBCollider, ColliderUpdateTag>()) {
		auto* t = registry.GetComponent<No::TransformComponent>(e);
		auto* b = registry.GetComponent<No::OBBCollider>(e);

		b->extents = t->scale;
	}
}
