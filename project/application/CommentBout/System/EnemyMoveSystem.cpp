#include "stdafx.h"
#include "EnemyMoveSystem.h"
#include "application/CommentBout/Component/EnemyComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/GameTag.h"

namespace {
No::Vector3 NormalizeOrDefault(const No::Vector3& v, const No::Vector3& fallback) {
	if (v.LengthSquared() <= 0.000001f) {
		return fallback;
	}
	return v.Normalize();
}
}

void EnemyMoveSystem::Update(No::Registry& registry, float deltaTime)
{
	auto view = registry.View<CBRailEnemyTag, EnemyComponent, HealthComponent, No::TransformComponent>();
	for (auto entity : view) {
		if (registry.Has<CBBossTag>(entity)) {
			continue;
		}
		auto* enemy = registry.GetComponent<EnemyComponent>(entity);
		auto* health = registry.GetComponent<HealthComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		if (!enemy || !health || !transform) {
			continue;
		}
		if (health->isDead || health->hp <= 0) {
			continue;
		}

		const No::Vector3 moveDir = NormalizeOrDefault(enemy->moveDirection, No::Vector3(0.0f, 0.0f, -1.0f));
		transform->translate += moveDir * enemy->moveSpeed * deltaTime;
	}
}
