#include "TransformComponent.h"

namespace NoEngine {
Math::Matrix4x4 Component::TransformComponent::MakeAffineMatrix4x4(ECS::Registry& registry) const {
    Math::Matrix4x4 result;
    result.MakeAffine(scale, rotation, translate);

    if (parent != ECS::INVALID_ENTITY) {
        if (auto* parentTransform = registry.GetComponent<Component::TransformComponent>(parent)) {
            return result * parentTransform->MakeAffineMatrix4x4(registry);
        }    
    }

    return result;
}

Math::Vector3 Component::TransformComponent::GetWorldScale(ECS::Registry& registry) const {
    Math::Vector3 result = scale;
    if (parent != ECS::INVALID_ENTITY) {
        if (auto* parentTransform = registry.GetComponent<Component::TransformComponent>(parent)) {
            Math::Vector3 parentScale = parentTransform->GetWorldScale(registry);
            return Math::Vector3(result.x * parentScale.x, result.y * parentScale.y, result.z * parentScale.z);
        }
    }
    return result;
}

Math::Quaternion Component::TransformComponent::GetWorldRotation(ECS::Registry& registry) const {
    Math::Quaternion result = rotation;
    if (parent != ECS::INVALID_ENTITY) {
        if (auto* parentTransform = registry.GetComponent<Component::TransformComponent>(parent)) {
            Math::Quaternion parentRotation = parentTransform->GetWorldRotation(registry);
            return result * parentRotation;
        }
    }
    return result;
}
}

REFLECT_STRUCT_BEGIN(NoEngine::Component::TransformComponent, "Transform")
REFLECT_FIELD(translate),
REFLECT_FIELD(rotation),
REFLECT_FIELD(scale),
REFLECT_FIELD(parent)
REFLECT_STRUCT_END(NoEngine::Component::TransformComponent)