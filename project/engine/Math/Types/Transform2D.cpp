#include "Transform2D.h"

namespace NoEngine {
Matrix3x3 Transform2D::MakeAffineMatrix3x3() {
    Matrix3x3 result;
    result.MakeAffine(scale, rotation, translate);
    if (parent) {
        return result * parent->MakeAffineMatrix3x3();
    }
    return result;
}
}
