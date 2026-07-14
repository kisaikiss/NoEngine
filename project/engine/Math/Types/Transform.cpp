#include "Transform.h"

namespace NoEngine {

Math::Matrix4x4 Transform::MakeAffineMatrix4x4(ECS::Registry& registry) const {
    static_cast<void>(registry);
    Math::Matrix4x4 result;
    result.MakeAffine(scale, rotation, translate);
  
    return result;
}

Math::Matrix4x4 Transform::MakeAffineMatrix4x4() const {
    Math::Matrix4x4 result;
    result.MakeAffine(scale, rotation, translate);

    return result;
}

Math::Vector3 Transform::GetWorldPosition(ECS::Registry& registry) const {
    return MakeAffineMatrix4x4(registry).GetTranslate();
}
}

