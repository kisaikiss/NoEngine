#include "CollisionSystem.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "../Component/ColliderComponent.h"
#include <algorithm>

using namespace NoEngine::MathCalculations;

void CollisionSystem::Update(No::Registry& registry, float deltaTime)
{
	(void)deltaTime;
	UpdateCollider(registry);
	auto sphereView = registry.View<SphereColliderComponent>();
	auto boxView = registry.View<BoxColliderComponent>();
	//球
	for (auto entity1 : sphereView)
	{
		if (!registry.Has<SphereColliderComponent>(entity1))continue;
		auto* a = registry.GetComponent<SphereColliderComponent>(entity1);
		a->isCollied = false;
		//球
		for (auto entity2 : sphereView)
		{
			if (entity1 == entity2)continue;
			auto* b = registry.GetComponent<SphereColliderComponent>(entity2);
			b->isCollied = false;
			if ((a->colliderType & b->collideMask) == 0 || (b->colliderType & a->collideMask) == 0) continue;

			if (CheckSphereToSphere(a->center, b->center, a->worldRadius, b->worldRadius))
			{
				a->isCollied = true;
				b->isCollied = true;
				a->colliedWith = static_cast<ColliderMask>(b->colliderType);
				b->colliedWith = static_cast<ColliderMask>(a->colliderType);
				a->colliedEntity = entity2;
				b->colliedEntity = entity1;
			}
		}
	}
	//ボックス
	//for (auto entity1 : boxView)
	//{
	//	if (!registry.Has<BoxColliderComponent>(entity1))continue;
	//	auto* a = registry.GetComponent<BoxColliderComponent>(entity1);
	//	a->isCollied = false;
	//	//ボックス
	//	for (auto entity2 : boxView)
	//	{
	//		if (entity1 == entity2)continue;
	//		auto* b = registry.GetComponent<BoxColliderComponent>(entity2);
	//
	//		if (CheckBoxToBox(a->center, b->center, a->worldSize, b->worldSize))
	//		{
	//			a->isCollied = true;
	//			b->isCollied = true;
	//		}
	//	}
	//	//球
	//	for (auto entity2 : sphereView)
	//	{
	//		auto* b = registry.GetComponent<SphereColliderComponent>(entity2);
	//		if (CheckBoxToSphere(a->center, b->center, a->worldSize, b->worldRadius))
	//		{
	//			a->isCollied = true;
	//			b->isCollied = true;
	//		}
	//	}
	//}
}

void CollisionSystem::UpdateCollider(No::Registry& registry)
{
	auto sphereView = registry.View<SphereColliderComponent, No::TransformComponent>();
	auto boxView = registry.View<BoxColliderComponent, No::TransformComponent>();

	//球状コライダー更新
	for (auto entity : sphereView)
	{
		if (!registry.Has<SphereColliderComponent>(entity))continue;

		auto* sphereA = registry.GetComponent<SphereColliderComponent>(entity);
		auto* transformA = registry.GetComponent<No::TransformComponent>(entity);

		sphereA->center = transformA->translate;

		float maxScale = std::max({ std::abs(transformA->scale.x), std::abs(transformA->scale.y), std::abs(transformA->scale.z) });
		sphereA->worldRadius = sphereA->radius * maxScale;
	}
	//ボックスコライダー更新
	for (auto entity : boxView)
	{
		if (!registry.Has<BoxColliderComponent>(entity))continue;

		auto* box = registry.GetComponent<BoxColliderComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);

		box->center = transform->translate;

		box->worldSize.x = std::abs(transform->scale.x) * box->size.x;
		box->worldSize.y = std::abs(transform->scale.y) * box->size.y;
		box->worldSize.z = std::abs(transform->scale.z) * box->size.z;
	}
}

bool CollisionSystem::CheckSphereToSphere(const NoEngine::Vector3& center1, const NoEngine::Vector3& center2, const float radius1, const float radius2)
{
	NoEngine::Vector3 diff = center1 - center2;
	float distanceSq = LengthSquared(diff);
	float radiusSum = radius1 + radius2;
	return distanceSq <= radiusSum * radiusSum;
}

bool CollisionSystem::CheckBoxToBox(const NoEngine::Vector3& center1, const NoEngine::Vector3& center2, const NoEngine::Vector3& size1, const NoEngine::Vector3& size2)
{
	auto minA = center1 - size1 * 0.5f;
	auto maxA = center1 + size1 * 0.5f;

	auto minB = center2 - size2 * 0.5f;
	auto maxB = center2 + size2 * 0.5f;

	return minA.x <= maxB.x && maxA.x >= minB.x &&
		minA.y <= maxB.y && maxA.y >= minB.y &&
		minA.z <= maxB.z && maxA.z >= minB.z;
}

bool CollisionSystem::CheckBoxToSphere(const NoEngine::Vector3& center1, const NoEngine::Vector3& center2, const NoEngine::Vector3& size, const float radius)
{
	auto min = center1 - size * 0.5f;
	auto max = center1 + size * 0.5f;

	// AABBと球の最近接点を求める
	NoEngine::Vector3 closetPoint = {
		std::clamp(center2.x, min.x, max.x),
		std::clamp(center2.y, min.y, max.y),
		std::clamp(center2.z, min.z, max.z)
	};
	// 最近接点と球の中心との距離を求める
	float distance = LengthSquared(closetPoint - center2);

	return distance <= radius * radius;
}
