#include "Transform.h"

namespace NoEngine {

Matrix4x4 Transform::MakeAffineMatrix4x4() {
    Matrix4x4 result;
    result.MakeAffine(scale, rotation, translate);
    if (parent) {
        return result * parent->MakeAffineMatrix4x4();
    }
    return result;
}

Vector3 Transform::GetWorldPosition() {
    return MakeAffineMatrix4x4().GetTranslate();
}
}

