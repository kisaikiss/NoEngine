#include "EnemyBulletControlSystem.h"
#include "application/TD2_3/Component/Enemy/EnemyBulletComponent.h"
#include "application/TD2_3/Component/ColliderComponent.h"
#include "application/TD2_3/tag.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "engine/Math/Types/Calculations/QuaternionCalculations.h"

void EnemyBulletControlSystem::Update(No::Registry& registry, float deltaTime) {
	using namespace NoEngine;
	using namespace MathCalculations;
	auto view = registry.View<EnemyBulletComponent>();
	//bool hasEntities = (view.begin() != view.end());
	//if (!hasEntities) return;
	for (auto entity : view) {
		auto* t = registry.GetComponent<No::TransformComponent>(entity);
		auto* b = registry.GetComponent<EnemyBulletComponent>(entity);
		auto* c = registry.GetComponent<SphereColliderComponent>(entity);

		if (c->isCollied) {
			b->isCollide = true;
		}

		t->translate += b->velocity * deltaTime;
		
		if (b->isCollide) {
			b->t +=  3.f * deltaTime;
			t->scale = No::Lerp(t->scale, { 0.f,0.f,0.f }, b->t);
			if (b->t > 1.f) {
				auto* d = registry.GetComponent<DeathFlag>(entity);
				d->isDead = true;
			}
		}

		if (t->translate.x > 14.f || t->translate.x < -14.f ||
			t->translate.y > 7.f || t->translate.y < -7.f) {
			auto* d = registry.GetComponent<DeathFlag>(entity);
			d->isDead = true;
		}
	}
}
