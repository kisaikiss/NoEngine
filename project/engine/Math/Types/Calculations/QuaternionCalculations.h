#pragma once
#include "../Quaternion.h"

namespace NoEngine {

struct Math::Vector3;
struct Math::Matrix4x4;
namespace MathCalculations {
Math::Quaternion IdentityQuaternion();

Math::Quaternion Conjugate(const Math::Quaternion& quaternion);

Math::Quaternion Multiply(const Math::Quaternion& q1, const Math::Quaternion& q2);

float Dot(const Math::Quaternion& q1, const Math::Quaternion& q2);

Math::Quaternion Normalize(const Math::Quaternion& q);

float Norm(const Math::Quaternion& q);

Math::Quaternion Inverse(const Math::Quaternion& q);

Math::Quaternion MakeRotateAxisAngleQuaternion(const Math::Vector3& axis, float angle);

Math::Vector3 RotateVector(const Math::Vector3& vector, const Math::Quaternion& quaternion);

Math::Matrix4x4 MakeRotateMatrix(const Math::Quaternion& quaternion);

Math::Quaternion Slerp(const Math::Quaternion& q0,const Math::Quaternion& q1, float t);
}
}