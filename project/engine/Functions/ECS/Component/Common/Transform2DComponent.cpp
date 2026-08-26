#include "Transform2DComponent.h"
#include "engine/Editor/ReflectionMacros.h"

namespace NoEngine {
namespace Component {
Math::Matrix3x3 Transform2DComponent::MakeAffineMatrix3x3(ECS::Registry& registry) const {
    Math::Matrix3x3 result;
    result.MakeAffine(scale, rotation, translate);
    if (parent != ECS::INVALID_ENTITY) {
        if (auto* parentTransform = registry.GetComponent<Component::Transform2DComponent>(parent)) {
            return result * parentTransform->MakeAffineMatrix3x3(registry);
        }
    }
    return result;
}

Math::Matrix4x4 Transform2DComponent::MakeAffineMatrix4x4(ECS::Registry& registry) const {
    Math::Matrix4x4 result;
    Math::Vector3 scale3 = { scale.x,scale.y, 1.0f };
    Math::Vector3 translate3 = { translate.x, translate.y, 0.0f };
    Math::Quaternion rotate4;

    rotate4.FromAxisAngle(Math::Vector3::FORWARD, rotation);

    result.MakeAffine(scale3, rotate4, translate3);
    if (parent != ECS::INVALID_ENTITY) {
        if (auto* parentTransform = registry.GetComponent<Component::Transform2DComponent>(parent)) {
            return result * parentTransform->MakeAffineMatrix4x4(registry);
        }
    }
    return result;
}

Math::Vector2 Transform2DComponent::GetWorldScale(ECS::Registry& registry) const {
    Math::Vector2 result = scale;
    if (parent != ECS::INVALID_ENTITY) {
        if (auto* parentTransform = registry.GetComponent<Component::Transform2DComponent>(parent)) {
            Math::Vector2 parentScale = parentTransform->GetWorldScale(registry);
            return Math::Vector2(result.x * parentScale.x, result.y * parentScale.y);
        }
    }
    return result;
}

float Transform2DComponent::GetWorldRotation(ECS::Registry& registry) const {
    float result = rotation;
    if (parent != ECS::INVALID_ENTITY) {
        if (auto* parentTransform = registry.GetComponent<Component::Transform2DComponent>(parent)) {
            result += parentTransform->GetWorldRotation(registry);
        }
    }
    return result;
}


}
}

REFLECT_STRUCT_BEGIN(NoEngine::Component::Transform2DComponent, "Transform")
REFLECT_FIELD(translate),
REFLECT_FIELD(rotation),
REFLECT_FIELD(scale),
REFLECT_FIELD(parent)
REFLECT_STRUCT_END(NoEngine::Component::Transform2DComponent)