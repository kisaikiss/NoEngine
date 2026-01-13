#include "Vector2Calculations.h"

namespace NoEngine {
namespace MathCalculations {

float Length(const Vector2& v) {
	return std::sqrtf(v.x * v.x + v.y * v.y);
}

float LengthSquared(const Vector2& v) {
	return v.x * v.x + v.y * v.y;
}

float Dot(const Vector2& v1, const Vector2& v2) {
	return v1.x * v2.x + v1.y * v2.y;
}

float Closs(const Vector2& v1, const Vector2& v2) {
	return v1.x * v2.y - v1.y * v2.x;
}

Vector2 Normalize(const Vector2& v) {
	float length = Length(v);
	assert(length != 0);
	return Vector2(v.x / length, v.y / length);
}

}
}