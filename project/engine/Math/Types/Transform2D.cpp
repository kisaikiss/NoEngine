#include "Transform2D.h"

namespace NoEngine {
Math::Matrix3x3 Transform2D::MakeAffineMatrix3x3(ECS::Registry& registry) const {
    static_cast<void>(registry);
    Math::Matrix3x3 result;
    result.MakeAffine(scale, rotation, translate);
   
    return result;
}

Math::Matrix4x4 Transform2D::MakeAffineMatrix4x4(ECS::Registry& registry) const {
    static_cast<void>(registry);
    Math::Matrix4x4 result;
    Math::Vector3 scale3 = { scale.x,scale.y, 1.0f };
    Math::Vector3 translate3 = { translate.x, translate.y, 0.0f };
    Math::Quaternion rotate4;

    rotate4.FromAxisAngle(Math::Vector3::FORWARD, rotation);

    result.MakeAffine(scale3, rotate4, translate3);

    return result;
}
Math::Vector2 Transform2D::GetWorldPosition(ECS::Registry& registry) const {
    return MakeAffineMatrix3x3(registry).GetTranslate();
}
}
