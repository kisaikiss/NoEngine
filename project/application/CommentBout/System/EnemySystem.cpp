#include "stdafx.h"
#include "EnemySystem.h"

#include "application/CommentBout/Component/EnemyComponent.h"
#include "application/CommentBout/Component/AttackDamageComponent.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Utility/CBCollisionMask.h"
#include "application/TestApp/Component/Collider2DComponent.h"
#include "application/TestApp/Component/Collider3DComponent.h"

namespace {
No::Vector3 NormalizeOrDefault(const No::Vector3& v, const No::Vector3& fallback) {
	if (v.LengthSquared() <= 0.000001f) {
		return fallback;
	}
	return v.Normalize();
}
}

void EnemySystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<CBRailEnemyTag, EnemyComponent, No::TransformComponent, TestApp::Collider3DComponent>();
	for (auto entity : view) {
		auto* enemy = registry.GetComponent<EnemyComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* collider3D = registry.GetComponent<TestApp::Collider3DComponent>(entity);
		if (!enemy || !transform || !collider3D) {
			continue;
		}

		const No::Vector3 moveDir = NormalizeOrDefault(enemy->moveDirection, No::Vector3(0.0f, 0.0f, -1.0f));
		transform->translate += moveDir * enemy->moveSpeed * deltaTime;

		int attackDamage = 1;
		bool isHitByPlayerAttack = false;
		if (collider3D->isColliding) {
			auto* hit2D = registry.GetComponent<TestApp::Collider2DComponent>(collider3D->collidedEntity);
			if (hit2D && hit2D->collisionLayer == CommentBout::CollisionLayer::CBPlayerAttack) {
				isHitByPlayerAttack = true;
				auto* damage = registry.GetComponent<AttackDamageComponent>(collider3D->collidedEntity);
				if (damage) {
					attackDamage = std::max(1, damage->damage);
				}
			}
		}

		if (isHitByPlayerAttack && !enemy->wasCollidingWithAttack) {
			enemy->hp -= attackDamage;
			if (enemy->hp < 0) {
				enemy->hp = 0;
			}
		}
		enemy->wasCollidingWithAttack = isHitByPlayerAttack;

		if (enemy->hp <= 0) {
			registry.DestroyEntity(entity);
		}
	}
}
