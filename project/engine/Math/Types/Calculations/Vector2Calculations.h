#pragma once
#include "../Vector2.h"

namespace NoEngine {
namespace MathCalculations {
float Length(const Vector2& v);
float LengthSquared(const Vector2& v);
float Dot(const Vector2& a, const Vector2& b);
float Closs(const Vector2& v1, const Vector2& v2);
Vector2 Normalize(const Vector2& v);
}
}