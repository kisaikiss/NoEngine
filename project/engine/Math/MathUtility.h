#pragma once
#include "Types/Vector2.h"
#include "Types/Vector3.h"
#include "Types/Vector4.h"
#include "Types/Matrix4x4.h"

namespace NoEngine {
namespace Math {
Vector4 Clamp(const Vector4& v, const Vector4& min, const Vector4& max);
Vector3 Clamp(const Vector3& v, const Vector3& min, const Vector3& max);
Vector2 Clamp(const Vector2& v, const Vector2& min, const Vector2& max);

Vector2 WorldToScreen(const Vector3& p, const Matrix4x4& view, const Matrix4x4& projection);
}
}