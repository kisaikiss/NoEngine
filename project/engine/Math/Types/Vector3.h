#pragma once
namespace NoEngine {

struct Vector3 final {
public:
	float x, y, z;

	Vector3& operator+=(const Vector3& other);
	Vector3& operator-=(const Vector3& other);
	Vector3& operator*=(const float s);
	Vector3& operator/=(const float s);
};

Vector3 operator+(const Vector3& v1, const Vector3& v2);

Vector3 operator-(const Vector3& v1, const Vector3& v2);

Vector3 operator*(float s, const Vector3& v);

Vector3 operator*(const Vector3& v, float s);

Vector3 operator/(const Vector3& v, float s);

Vector3 operator+(const Vector3& v);

Vector3 operator-(const Vector3& v);
}