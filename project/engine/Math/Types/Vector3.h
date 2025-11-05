#pragma once
namespace NoEngine {

struct Vector3 final {
public:
	float x, y, z;

	Vector3& operator+=(const Vector3& other);
	Vector3& operator-=(const Vector3& other);
	Vector3& operator*=(const float s);
	Vector3& operator/=(const float s);

	[[nodiscard]] float Dot(const Vector3& other) const noexcept;
	[[nodiscard]] Vector3 Cross(const Vector3& other) const noexcept;
	[[nodiscard]] float Length() const noexcept;
	[[nodiscard]] float LengthSquared() const noexcept;
	[[nodiscard]] Vector3 Normalize() const;
};

Vector3 operator+(const Vector3& v1, const Vector3& v2);

Vector3 operator-(const Vector3& v1, const Vector3& v2);

Vector3 operator*(float s, const Vector3& v);

Vector3 operator*(const Vector3& v, float s);

Vector3 operator/(const Vector3& v, float s);

Vector3 operator+(const Vector3& v);

Vector3 operator-(const Vector3& v);
}