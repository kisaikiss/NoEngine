#pragma once
#include "ColliderComponent.h"
#include "../Types/Transform.h"

namespace NoEngine {
namespace Math {

struct CapsuleAABBCollision {
	bool hit = false;
	Vector3 closestOnCapsule;
	Vector3 closestOnBox;
	Vector3 normal;	// box -> capsule
	float penetration = 0.0f;
};

CapsuleAABBCollision TestCapsuleAABB(const Transform* capsuleTransform, const Math::CapsuleCollider* capsule, const Transform* aabbTransform, const Math::AABBCollider* aabb);

}

}