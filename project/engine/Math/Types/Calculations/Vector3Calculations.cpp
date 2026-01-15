#include "Vector3Calculations.h"

namespace NoEngine {
namespace MathCalculations {
float Dot(Vector3 v1, Vector3 v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vector3 Cross(const Vector3& v1, const Vector3& v2) {
	Vector3 result{};
	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;
	return result;
}

float Length(Vector3 vector3) {
	return sqrtf(powf(vector3.x, 2.0f) + powf(vector3.y, 2.0f) + powf(vector3.z, 2.0f));
}

float LengthSquared(const Vector3& v) {
	return powf(v.x, 2.0f) + powf(v.y, 2.0f) + powf(v.z, 2.0f);
}

Vector3 Normalize(const Vector3& vector3) {
	float length = Length(vector3);
	assert(length != 0);
	return Vector3(vector3.x / length, vector3.y / length, vector3.z / length);
}

Vector3 MakeOrthographicVector(Vector3& v1, Vector3& v2) {
	Vector3 result{};
	v2 = Normalize(v2);
	result.x = (v2.x * Dot(v1, v2));
	result.y = (v2.y * Dot(v1, v2));
	result.z = (v2.z * Dot(v1, v2));
	return result;
}
float Distance(const Vector3& v1, const Vector3& v2)
{
	return Length(v1 - v2);
}
}
}