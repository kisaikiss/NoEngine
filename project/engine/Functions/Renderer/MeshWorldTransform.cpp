#include "MeshWorldTransform.h"
#include "engine/Functions/ECS/Registry.h"
#include "engine/Functions/ECS/Component/Asset/AnimatorComponent.h"
#include "engine/Functions/ECS/Component/Common/BillboardComponent.h"
#include "engine/Functions/ECS/Component/Common/CameraComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include <cmath>

namespace NoEngine {
namespace {
using Math::Vector3;

Vector3 UnitOr(const Vector3& value, const Vector3& fallback) {
    const float lengthSquared = value.LengthSquared();
    if (!std::isfinite(lengthSquared) || lengthSquared < 1.0e-10f) return fallback;
    return value / std::sqrt(lengthSquared);
}

Vector3 Perpendicular(const Vector3& axis) {
    const Vector3 candidate = std::abs(axis.z) < 0.9f ? Vector3::FORWARD : Vector3::RIGHT;
    return UnitOr(candidate - axis * candidate.Dot(axis), Vector3::RIGHT);
}
}

Math::Matrix4x4 MakeBillboardWorldMatrix(const Math::Matrix4x4& placementWorld,
    const Math::Vector3& worldScale, const Component::BillboardComponent& billboard,
    const Component::CameraComponent* camera) {
    if (!billboard.enabled || !camera) return placementWorld;

    auto cameraWorld = camera->view;
    cameraWorld.Inverse();
    const Vector3 cameraForward = UnitOr(
        { cameraWorld.m[2][0], cameraWorld.m[2][1], cameraWorld.m[2][2] }, Vector3::FORWARD);
    const Vector3 cameraUp = UnitOr(
        { cameraWorld.m[1][0], cameraWorld.m[1][1], cameraWorld.m[1][2] }, Vector3::UP);
    const Vector3 axis = UnitOr(billboard.axis, Vector3::UP);
    Vector3 forward = billboard.facing == Component::BillboardFacing::ScreenAligned
        ? -cameraForward : camera->forGPU.worldPosition - placementWorld.GetTranslate();
    Vector3 up = billboard.facing == Component::BillboardFacing::ScreenAligned ? cameraUp : axis;

    if (billboard.constraint == Component::BillboardConstraint::AxisLocked) {
        // 軸上にカメラがある場合も、軸に直交する既定方向で安定した姿勢を作る。
        forward = UnitOr(forward - axis * forward.Dot(axis), Perpendicular(axis));
        up = axis;
    } else {
        forward = UnitOr(forward, -cameraForward);
        up = UnitOr(up - forward * up.Dot(forward), Perpendicular(forward));
    }

    const Vector3 right = UnitOr(up.Cross(forward), Perpendicular(forward));
    up = UnitOr(forward.Cross(right), up);
    const Math::Matrix4x4 orientation(right, up, forward, Vector3::ZERO);
    auto offset = billboard.rotationOffset;
    const float normSquared = offset.x * offset.x + offset.y * offset.y +
        offset.z * offset.z + offset.w * offset.w;
    if (!std::isfinite(normSquared) || normSquared < 1.0e-10f) offset = Math::Quaternion::IDENTITY;
    else offset.Normalize();

    Math::Matrix4x4 local;
    local.MakeAffine(worldScale, offset, Vector3::ZERO);
    auto result = local * orientation;
    const auto position = placementWorld.GetTranslate();
    result.m[3][0] = position.x;
    result.m[3][1] = position.y;
    result.m[3][2] = position.z;
    return result;
}

Math::Matrix4x4 MakeMeshWorldMatrix(ECS::Registry& registry, ECS::Entity entity,
    const Component::CameraComponent* renderCamera, const Component::CameraComponent* activeCamera) {
    const auto* transform = registry.GetComponent<Component::TransformComponent>(entity);
    if (!transform) return Math::Matrix4x4::IDENTITY;
    auto world = transform->MakeAffineMatrix4x4(registry);
    if (const auto* billboard = registry.GetComponent<Component::BillboardComponent>(entity);
        billboard && billboard->enabled) {
        const auto* camera = billboard->cameraSource == Component::BillboardCameraSource::ActiveCamera
            ? activeCamera : renderCamera;
        world = MakeBillboardWorldMatrix(world, transform->GetWorldScale(registry), *billboard, camera);
    }
    if (const auto* animator = registry.GetComponent<Component::AnimatorComponent>(entity)) {
        world = animator->local.MakeAffineMatrix4x4(registry) * world;
    }
    return world;
}
}
