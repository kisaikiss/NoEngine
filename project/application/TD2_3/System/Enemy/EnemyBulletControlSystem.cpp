#include "EnemyBulletControlSystem.h"
#include "application/TD2_3/Component/Enemy/EnemyBulletComponent.h"
#include "application/TD2_3/tag.h"

void EnemyBulletControlSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<EnemyBulletComponent>();
	if (view.Empty()) return;
	for (auto entity : view) {
		auto* t = registry.GetComponent<No::TransformComponent>(entity);
		auto* b = registry.GetComponent<EnemyBulletComponent>(entity);
		t->translate += b->velocity * deltaTime;
		if (t->translate.x > 14.f || t->translate.x < -14.f ||
			t->translate.y > 7.f || t->translate.y < -7.f) {
			auto* d = registry.GetComponent<DeathFlag>(entity);
			d->isDead = true;
		}
	}
}
