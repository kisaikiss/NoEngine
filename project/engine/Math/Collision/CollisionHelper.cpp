#include "CollisionHelper.h"

namespace NoEngine {
AABBWorld GetWorldAABB(const Transform* transform, const Math::AABBCollider* aabb) {
    AABBWorld result;
    Math::Vector3 worldPos = transform->GetWorldPosition();
    result.max = worldPos + aabb->max;
    result.min = worldPos + aabb->min;

    return result;
}

AABBWorld2D GetWorldAABB2D(const Transform2D* transform, const Math::AABBCollider2D* aabb) {
    AABBWorld2D result;
    Math::Vector2 worldPos = transform->translate;
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

SphereWorld GetWorldSphere(const Transform* transform, const Math::SphereCollider* sphere) {
    SphereWorld result;
    Math::Vector3 worldPos = transform->GetWorldPosition();
    result.center = worldPos;
    result.radius = sphere->radius;
    return result;
}
}
