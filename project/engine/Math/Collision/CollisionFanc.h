#pragma once
#include "ColliderComponent.h"
#include "MeshColliderLoader.h"
#include "../Types/Transform.h"
#include "../Types/Transform2D.h"

namespace NoEngine {
namespace Math {

struct CapsuleAABBCollision {
	bool hit = false;
	Vector3 closestOnCapsule;
	Vector3 closestOnBox;
	Vector3 normal;	// box -> capsule
	float penetration = 0.0f;
};

struct Collision2D {
	bool hit = false;
	Vector2 closestOnA;
	Vector2 closestOnB;
	Vector2 normal;	// B -> A
	float penetration = 0.0f;
};

CapsuleAABBCollision TestCapsuleAABB(const Transform* capsuleTransform, const Math::CapsuleCollider* capsule, const Transform* aabbTransform, const Math::AABBCollider* aabb);
Collision2D TestAABB2D(const Transform2D* transformA, const Math::AABBCollider2D* aabbA, const Transform2D* transformB, const Math::AABBCollider2D* aabbB);
Collision2D TestAABB2DHorizontal(const Transform2D* transformA, const Math::AABBCollider2D* aabbA, const Transform2D* transformB, const Math::AABBCollider2D* aabbB);
Collision2D TestAABB2DVertical(const Transform2D* transformA, const Math::AABBCollider2D* aabbA, const Transform2D* transformB, const Math::AABBCollider2D* aabbB);
bool IsCollision(const Vector2& positon, const AABBCollider2D* box, const Transform2D* transformA);
}

}