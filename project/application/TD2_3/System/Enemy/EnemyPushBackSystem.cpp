#include "EnemyPushBackSystem.h"
#include "../../tag.h"
#include "../../Component/ColliderComponent.h"

void EnemyPushBackSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto view = registry.View<EnemyTag, SphereColliderComponent>();
	for (auto entity1 : view) {
		if (!registry.Has<SphereColliderComponent>(entity1))continue;
		auto* a = registry.GetComponent<SphereColliderComponent>(entity1);
		for (auto entity2 : view) {
			if (entity1 == entity2 || !registry.Has<SphereColliderComponent>(entity2))continue;
			auto* b = registry.GetComponent<SphereColliderComponent>(entity2);
			// 衝突判定を行う
			if (CheckSphereToSphere(a->center, b->center, a->worldRadius, b->worldRadius)) {
				// 押し戻す
				PushBack(registry, entity1, entity2);
			}
		}
	}

}

bool EnemyPushBackSystem::CheckSphereToSphere(const No::Vector3& center1, const No::Vector3& center2, const float radius1, const float radius2) {
	No::Vector3 diff = center1 - center2;
	float distanceSq = diff.LengthSquared();
	float radiusSum = radius1 + radius2;
	return distanceSq <= radiusSum * radiusSum;
}

void EnemyPushBackSystem::PushBack(No::Registry& registry, No::Entity e1, No::Entity e2) {
	using namespace No;

	auto* transform1 = registry.GetComponent<No::TransformComponent>(e1);
	auto* transform2 = registry.GetComponent<No::TransformComponent>(e2);
	auto* colliderA = registry.GetComponent<SphereColliderComponent>(e1);
	auto* colliderB = registry.GetComponent<SphereColliderComponent>(e2);
	Vector3 posA = transform1->GetWorldPosition();
	Vector3 posB = transform2->GetWorldPosition();

	Vector3 delta = posB - posA;
	float dist = delta.Length();
	if (dist == 0.0f) return;


	float minDist = colliderA->worldRadius + colliderB->worldRadius;
	float penetration = minDist - dist;
	if (penetration <= 0.0f) return;

	Vector3 dir = delta / dist;
	Vector3 correction = dir * (penetration * 0.5f);

	transform1->translate -= correction;
	transform2->translate += correction;

}
