#include "stdafx.h"
#include "IngredientsControlSystem.h"
#include "application/TD2_3/Component/ColliderComponent.h"
#include "application/TD2_3/Component/Enemy/IngredientsComponent.h"
#include "application/TD2_3/tag.h"

void IngredientsControlSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<IngredientsComponent>();


	for (auto entity : view) {
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* b = registry.GetComponent<IngredientsComponent>(entity);
		b->velocity.y += b->skGravity * deltaTime;
		transform->translate += b->velocity * deltaTime;

		auto* c = registry.GetComponent<SphereColliderComponent>(entity);

		if (c->isCollied) {
			b->isCollide = true;
		}

		if (b->isCollide) {
			b->t += 3.f * deltaTime;
			transform->scale = No::Lerp(transform->scale, { 0.f,0.f,0.f }, b->t);
			if (b->t > 1.f) {
				auto* d = registry.GetComponent<DeathFlag>(entity);
				d->isDead = true;
			}
		}
	}
}
