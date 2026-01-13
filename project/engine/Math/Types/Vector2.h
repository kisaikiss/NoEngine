#pragma once
#include <stdlib.h>
namespace NoEngine {

struct Vector2 final {
	float x, y;

	Vector2& operator+=(const Vector2& other);
	Vector2& operator-=(const Vector2& other);
	Vector2& operator*=(const float s);
	Vector2& operator/=(const float s);

	[[nodiscard]] float Dot(const Vector2& other)const noexcept;
	[[nodiscard]] float Closs(const Vector2& other)const noexcept;
	[[nodiscard]] float Length() const noexcept;
	[[nodiscard]] float LengthSquared() const noexcept;
	[[nodiscard]] Vector2 Normalize() const;

	static const Vector2 ZERO;
	static const Vector2 RIGHT;
	static const Vector2 UNIT_SCALE;
};

Vector2 operator+(const Vector2& v1, const Vector2& v2);

Vector2 operator-(const Vector2& v1, const Vector2& v2);

Vector2 operator*(float s, const Vector2& v);

Vector2 operator*(const Vector2& v, float s);

Vector2 operator/(const Vector2& v, float s);
}