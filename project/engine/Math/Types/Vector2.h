#pragma once
namespace NoEngine {

struct Vector2 final{
	float x, y;

	Vector2& operator+=(const Vector2& other);
	Vector2& operator-=(const Vector2& other);
	Vector2& operator*=(const float s);
	Vector2& operator/=(const float s);
};

Vector2 operator+(const Vector2& v1, const Vector2& v2);

Vector2 operator-(const Vector2& v1, const Vector2& v2);

Vector2 operator*(float s, const Vector2& v);

Vector2 operator*(const Vector2& v, float s);

Vector2 operator/(const Vector2& v, float s);
}