#include "stdafx.h"
#include "EnemySystem.h"

#include "application/CommentBout/Component/EnemyComponent.h"
#include "application/CommentBout/Component/AttackDamageComponent.h"
#include "application/CommentBout/Component/PlayerHealthComponent.h"
#include "application/CommentBout/Component/PlayerHitboxComponent.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Utility/CBCollisionMask.h"
#include "application/TestApp/Component/Collider2DComponent.h"
#include "application/TestApp/Component/Collider3DComponent.h"
#include "application/TestApp/Utility/CollisionAlgorithms.h"
#include "engine/Functions/ECS/Component/MaterialComponent.h"
#include "engine/Functions/Renderer/Primitive.h"

namespace No {
using ::NoEngine::Primitive;
}

namespace {
No::Vector3 NormalizeOrDefault(const No::Vector3& v, const No::Vector3& fallback) {
	if (v.LengthSquared() <= 0.000001f) {
		return fallback;
	}
	return v.Normalize();
}

bool CheckEnemyVsPlayerHitbox(const TestApp::Collider3DComponent& enemyCollider, const TestApp::Collider3DComponent& playerCollider) {
	if (enemyCollider.shapeType == TestApp::ShapeType3D::Box && playerCollider.shapeType == TestApp::ShapeType3D::Box) {
		return TestApp::CollisionAlgorithms::CheckAABB3DAABB3D(
			enemyCollider.worldPosition,
			enemyCollider.worldBoxSize,
			playerCollider.worldPosition,
			playerCollider.worldBoxSize
		);
	}
	if (enemyCollider.shapeType == TestApp::ShapeType3D::Sphere && playerCollider.shapeType == TestApp::ShapeType3D::Sphere) {
		return TestApp::CollisionAlgorithms::CheckSphereSphere(
			enemyCollider.worldPosition,
			enemyCollider.worldRadius,
			playerCollider.worldPosition,
			playerCollider.worldRadius
		);
	}
	if (enemyCollider.shapeType == TestApp::ShapeType3D::Sphere && playerCollider.shapeType == TestApp::ShapeType3D::Box) {
		return TestApp::CollisionAlgorithms::CheckSphereAABB3D(
			enemyCollider.worldPosition,
			enemyCollider.worldRadius,
			playerCollider.worldPosition,
			playerCollider.worldBoxSize
		);
	}
	return TestApp::CollisionAlgorithms::CheckSphereAABB3D(
		playerCollider.worldPosition,
		playerCollider.worldRadius,
		enemyCollider.worldPosition,
		enemyCollider.worldBoxSize
	);
}

void DrawColliderDebug(const TestApp::Collider3DComponent& collider, const NoEngine::Math::Color& color) {
	if (collider.shapeType == TestApp::ShapeType3D::Box) {
		No::Primitive::DrawCube(collider.worldPosition, collider.worldBoxSize, color);
	} else {
		No::Primitive::DrawSphere(collider.worldPosition, collider.worldRadius, color, 10, 10);
	}
}
}

void EnemySystem::Update(No::Registry& registry, float deltaTime) {
	static bool drawColliderDebug = true;
#ifdef USE_IMGUI
	ImGui::Begin("Enemy Debug");
	ImGui::Checkbox("Draw Enemy Colliders", &drawColliderDebug);
	ImGui::Separator();
#endif

	auto view = registry.View<CBRailEnemyTag, EnemyComponent, No::TransformComponent, No::MaterialComponent, TestApp::Collider3DComponent>();
	for (auto entity : view) {
		auto* enemy = registry.GetComponent<EnemyComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* material = registry.GetComponent<No::MaterialComponent>(entity);
		auto* collider3D = registry.GetComponent<TestApp::Collider3DComponent>(entity);
		if (!enemy || !transform || !material || !collider3D) {
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
			enemy->lastDamageTaken = attackDamage;
			enemy->damageFlashTimer = enemy->damageFlashDuration;
			if (enemy->hp < 0) {
				enemy->hp = 0;
			}
		}
		enemy->wasCollidingWithAttack = isHitByPlayerAttack;

		bool isTouchingPlayerHitbox = false;
		auto hitboxView = registry.View<CBPlayerHitboxTag, PlayerHitboxComponent, TestApp::Collider3DComponent>();
		for (auto hitboxEntity : hitboxView) {
			auto* hitbox = registry.GetComponent<PlayerHitboxComponent>(hitboxEntity);
			auto* playerCollider = registry.GetComponent<TestApp::Collider3DComponent>(hitboxEntity);
			if (!hitbox || !playerCollider) {
				continue;
			}
			if (CheckEnemyVsPlayerHitbox(*collider3D, *playerCollider)) {
				isTouchingPlayerHitbox = true;
				auto* health = registry.GetComponent<PlayerHealthComponent>(hitbox->playerEntity);
				if (health && !health->isDead && health->invincibleTime <= 0.0f && !enemy->wasCollidingWithPlayer) {
					health->hp -= 1;
					if (health->hp < 0) {
						health->hp = 0;
					}
					health->lastDamageTaken = 1;
					health->invincibleTime = health->invincibleDuration;
					if (health->hp <= 0) {
						health->isDead = true;
					}
				}
			}
		}
		enemy->wasCollidingWithPlayer = isTouchingPlayerHitbox;

		if (enemy->damageFlashTimer > 0.0f) {
			enemy->damageFlashTimer -= deltaTime;
			material->color = { 1.0f, 0.2f, 0.2f, 1.0f };
		} else {
			enemy->damageFlashTimer = 0.0f;
			material->color = { 1.0f, 1.0f, 1.0f, 1.0f };
		}

#ifdef USE_IMGUI
		ImGui::Text("Enemy %llu hp=%d/%d atkHit=%s flash=%.2f playerHit=%s", static_cast<unsigned long long>(entity), enemy->hp, enemy->maxHp, isHitByPlayerAttack ? "true" : "false", enemy->damageFlashTimer, isTouchingPlayerHitbox ? "true" : "false");
		ImGui::Text("Collider pos(%.2f, %.2f, %.2f)", collider3D->worldPosition.x, collider3D->worldPosition.y, collider3D->worldPosition.z);
		if (collider3D->shapeType == TestApp::ShapeType3D::Box) {
			ImGui::Text("Collider box(%.2f, %.2f, %.2f)", collider3D->worldBoxSize.x, collider3D->worldBoxSize.y, collider3D->worldBoxSize.z);
		} else {
			ImGui::Text("Collider radius=%.2f", collider3D->worldRadius);
		}
		ImGui::Separator();
#endif

		if (drawColliderDebug) {
			const NoEngine::Math::Color color = collider3D->isColliding ? NoEngine::Math::Color(1.0f, 0.0f, 0.0f, 1.0f) : NoEngine::Math::Color(1.0f, 1.0f, 1.0f, 1.0f);
			DrawColliderDebug(*collider3D, color);
		}

		if (enemy->hp <= 0) {
			registry.DestroyEntity(entity);
		}
	}

#ifdef USE_IMGUI
	ImGui::End();
#endif
}
