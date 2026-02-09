#pragma once
#include "../Vector2.h"

namespace NoEngine {
namespace MathCalculations {
float Length(const Math::Vector2& v);
float LengthSquared(const Math::Vector2& v);
float Dot(const Math::Vector2& a, const Math::Vector2& b);
float Closs(const Math::Vector2& v1, const Math::Vector2& v2);
Math::Vector2 Normalize(const Math::Vector2& v);
}
}