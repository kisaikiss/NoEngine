#include "Vector3.h"
#include "Calculations/Vector3Calculations.h"

namespace NoEngine {
namespace Math {
const Vector3 Vector3::ZERO(0.f, 0.f, 0.f);
const Vector3 Vector3::RIGHT(1.f, 0.f, 0.f);
const Vector3 Vector3::UP(0.f, 1.f, 0.f);
const Vector3 Vector3::FORWARD(0.f, 0.f, 1.f);
const Vector3 Vector3::UNIT_SCALE(1.f, 1.f, 1.f);

Vector3 operator+(const Vector3& v1, const Vector3& v2) {
	return Vector3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

Vector3 operator-(const Vector3& v1, const Vector3& v2) {
	return Vector3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

Vector3 operator*(float s, const Vector3& v) {
	return Vector3(v.x * s, v.y * s, v.z * s);
}

Vector3 operator*(const Vector3& v, float s) {
	return s * v;
}

Vector3 operator/(const Vector3& v, float s) {
	return Vector3(v * (1.0f / s));
}

Vector3 operator+(const Vector3& v) {
	return v;
}

Vector3 operator-(const Vector3& v) {
	return Vector3(-v.x, -v.y, -v.z);
}

bool operator==(const Vector3& v1, const Vector3& v2) {
	return (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z);
}

Vector3& Vector3::operator+=(const Vector3& other) {
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

Vector3& Vector3::operator-=(const Vector3& other) {
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}

Vector3& Vector3::operator*=(const float s) {
	x *= s;
	y *= s;
	z *= s;
	return *this;
}

Vector3& Vector3::operator/=(const float s) {
	x /= s;
	y /= s;
	z /= s;
	return *this;
}

Vector3& Vector3::operator=(const float s) {
	x = s;
	y = s;
	z = s;
	return *this;
}

float Vector3::Dot(const Vector3& other) const noexcept {
	return MathCalculations::Dot(*this, other);
}

Vector3 Vector3::Cross(const Vector3& other) const noexcept {
	return MathCalculations::Cross(*this, other);
}

float Vector3::Length() const noexcept {
	return MathCalculations::Length(*this);
}

float Vector3::LengthSquared() const noexcept {
	return MathCalculations::LengthSquared(*this);
}

float Vector3::Distance(const Vector3& other) const noexcept {
	return MathCalculations::Distance(*this, other);
}

Vector3 Vector3::Normalize() const {
	return MathCalculations::Normalize(*this);
}

Vector3 Vector3::MakeOrthographicVector(const Vector3& other) const noexcept {
	return MathCalculations::MakeOrthographicVector(other, *this);
}
}
}