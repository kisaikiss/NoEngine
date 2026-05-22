#pragma once
#include "engine/Math/MathInclude.h"
namespace NoEngine {
namespace Random {

float GetRandomValNormalized();
float GetRandomVal(float min, float max);
Math::Vector3 GetRandomVal(const Math::Vector3& min, const Math::Vector3& max);
Math::Vector2 GetRandomVal(const Math::Vector2& min, const Math::Vector2& max);
}
}