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
}

REFLECT_STRUCT_BEGIN(NoEngine::Component::TransformComponent, "Transform")
REFLECT_FIELD(translate),
REFLECT_FIELD(rotation),
REFLECT_FIELD(scale),
REFLECT_FIELD(parent)
REFLECT_STRUCT_END(NoEngine::Component::TransformComponent)