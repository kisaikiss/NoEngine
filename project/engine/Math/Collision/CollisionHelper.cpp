#include "CollisionHelper.h"

namespace NoEngine {
AABBWorld GetWorldAABB(const Transform* transform, const Math::AABBCollider* aabb, ECS::Registry& registry) {
    AABBWorld result;
    Math::Vector3 worldPos = transform->GetWorldPosition(registry);
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

CapsuleWorld GetWorldCapsule(const Transform* transform, const Math::CapsuleCollider* capsule, ECS::Registry& registry) {
    CapsuleWorld result;
    Math::Vector3 worldPos = transform->GetWorldPosition(registry);
    result.p0 = worldPos + capsule->localP0;
    result.p1 = worldPos + capsule->localP1;

    result.radius = capsule->radius;

    return result;
}

SphereWorld GetWorldSphere(const Transform* transform, const Math::SphereCollider* sphere, ECS::Registry& registry) {
    SphereWorld result;
    Math::Vector3 worldPos = transform->GetWorldPosition(registry);
    result.center = worldPos;
    result.radius = sphere->radius;
    return result;
}

Math::AABBCollider ComputeCapsuleWorldBounds(const Transform* transform, const Math::CapsuleCollider* capsule, ECS::Registry& registry) {
    // カプセルのローカル始点・終点をワールド変換
    CapsuleWorld world = GetWorldCapsule(transform, capsule, registry);

    Math::AABBCollider box;
    box.Expand(world.p0);
    box.Expand(world.p1);
    box.min -= Math::Vector3(capsule->radius, capsule->radius, capsule->radius);
    box.max += Math::Vector3(capsule->radius, capsule->radius, capsule->radius);
    return box;
}
}
