#include "Transform.h"

namespace NoEngine {

Math::Matrix4x4 Transform::MakeAffineMatrix4x4() {
    Math::Matrix4x4 result;
    result.MakeAffine(scale, rotation, translate);
    if (parent) {
        return result * parent->MakeAffineMatrix4x4();
    }
    return result;
}

Math::Vector3 Transform::GetWorldPosition() {
    return MakeAffineMatrix4x4().GetTranslate();
}
}

