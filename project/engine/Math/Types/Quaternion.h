#pragma once
#include "Matrix3x3.h"
#include "Vector3.h"

namespace NoEngine {
namespace Math {
struct Matrix4x4;
struct Quaternion {
	float x;
	float y;
	float z;
	float w;
	Quaternion() = default;
	Quaternion(float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W) {}

	explicit Quaternion(const Matrix3x3& rot) { this->FromRotationMatrix(rot); }
	explicit Quaternion(const Matrix4x4& rot) { this->FromRotationMatrix(rot); }

	void Conjugate();
	void Normalize();
	void Inverse();
	float Norm();

	void FromRotationMatrix(const Matrix3x3& rotation);
	void FromRotationMatrix(const Matrix4x4& rotation);
	void FromAxisAngle(const Vector3& axis, float angle);
	Vector3 RotateVector(const Vector3& vector);
	void LookRotation(const Vector3& forward, const Vector3& up);

	static Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t);

	static const Quaternion ZERO;
	static const Quaternion IDENTITY;
};

Quaternion operator+(const Quaternion& q1, const Quaternion& q2);

Quaternion operator-(const Quaternion& q);

Quaternion operator*(const Quaternion& q1, const Quaternion& q2);

Quaternion operator*(const Quaternion& q, float s);

Quaternion operator*(float s, const Quaternion& q);

Quaternion operator-(const Quaternion& q0, const Quaternion& q1);
}
}