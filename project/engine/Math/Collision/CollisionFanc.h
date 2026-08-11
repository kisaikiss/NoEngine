#pragma once
#include "ColliderComponent.h"
#include "MeshColliderLoader.h"
#include "../Types/Transform.h"
#include "../Types/Transform2D.h"
#include "CollisionHelper.h"

namespace NoEngine {
namespace Math {

struct CapsuleAABBCollision {
	bool hit = false;
	Vector3 closestOnCapsule;
	Vector3 closestOnBox;
	Vector3 normal;	// box -> capsule
	float penetration = 0.0f;
};

struct CapsuleSphereCollision {
	bool hit = false;
	Vector3 closestOnCapsule;
	Vector3 closestOnSphere;
	Vector3 normal;	// sphere -> capsule
	float penetration = 0.0f;
};

struct CapsuleTriangleCollision {
	bool hit = false;
	Vector3 closestOnCapsule;
	Vector3 closestOnTriangle;
	Vector3 normal;	// triangle -> capsule
	float penetration = 0.0f;
};

struct SpherePushResult {
	bool hit = false;
	Vector3 normal;      // triangle -> sphere
	float penetration = 0.0f;
};

struct Collision2D {
	bool hit = false;
	Vector2 closestOnA;
	Vector2 closestOnB;
	Vector2 normal;	// B -> A
	float penetration = 0.0f;
};


SpherePushResult TestSphereTriangle(const Vector3& center, float radius, const TriangleCollider& triangle);

Math::Vector3 ClosestPointOnTriangle(const Math::Vector3& p, const Math::Vector3& a, const Math::Vector3& b, const Math::Vector3& c);
CapsuleAABBCollision TestCapsuleAABB(const Transform* capsuleTransform, const Math::CapsuleCollider* capsule, const Transform* aabbTransform, const Math::AABBCollider* aabb, ECS::Registry& registry);

// カメラのレイキャストや弾丸判定など、CapsuleColliderコンポーネントを持たない用途向け。
CapsuleTriangleCollision TestSegmentTriangle(
	const Vector3& segA, const Vector3& segB, float radius,const TriangleCollider& triangle);

// CapsuleColliderコンポーネントを持つ用途向け
CapsuleTriangleCollision TestCapsuleTriangle(
	const Transform* capsuleTransform, const Math::CapsuleCollider* capsule, const Math::TriangleCollider& triangle, ECS::Registry& registry);

CapsuleSphereCollision TestCapsuleSphere(const Transform* capsuleTransform, const Math::CapsuleCollider* capsule, const Transform* sphereTransform, const Math::SphereCollider* sphere, ECS::Registry& registry);

Collision2D TestAABB2D(const Transform2D* transformA, const Math::AABBCollider2D* aabbA, const Transform2D* transformB, const Math::AABBCollider2D* aabbB);
Collision2D TestAABB2DHorizontal(const Transform2D* transformA, const Math::AABBCollider2D* aabbA, const Transform2D* transformB, const Math::AABBCollider2D* aabbB);
Collision2D TestAABB2DVertical(const Transform2D* transformA, const Math::AABBCollider2D* aabbA, const Transform2D* transformB, const Math::AABBCollider2D* aabbB);
bool IsCollision(const Vector2& positon, const AABBCollider2D* box, const Transform2D* transformA);
}

}