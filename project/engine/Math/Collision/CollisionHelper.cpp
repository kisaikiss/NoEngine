#include "CollisionHelper.h"

namespace NoEngine {
AABBWorld GetWorldAABB(const Transform* transform, const Math::AABBCollider* aabb) {
    AABBWorld result;
    Math::Vector3 worldPos = transform->GetWorldPosition();
    result.max = worldPos + aabb->max;
    result.min = worldPos + aabb->min;

    return result;
}

CapsuleWorld GetWorldCapsule(const Transform* transform, const Math::CapsuleCollider* capsule) {
    CapsuleWorld result;
    Math::Vector3 worldPos = transform->GetWorldPosition();
    result.p0 = worldPos + capsule->localP0;
    result.p1 = worldPos + capsule->localP1;

    result.radius = capsule->radius;

    return result;
}
}
