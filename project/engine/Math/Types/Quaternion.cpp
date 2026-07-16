#include "Quaternion.h"
#include "Calculations/QuaternionCalculations.h"

#include "Matrix4x4.h"

namespace NoEngine {
namespace Math {
const Quaternion Quaternion::ZERO(0, 0, 0, 0);
const Quaternion Quaternion::IDENTITY(0, 0, 0, 1);

void Quaternion::Conjugate() {
	*this = MathCalculations::Conjugate(*this);
}

void Quaternion::Normalize() {
	*this = MathCalculations::Normalize(*this);
}

void Quaternion::Inverse() {
	*this = MathCalculations::Inverse(*this);
}

float Quaternion::Norm() {
	return MathCalculations::Norm(*this);
}

void Quaternion::FromRotationMatrix(const Matrix3x3& rotation) {
	float trace = rotation.m[0][0] + rotation.m[1][1] + rotation.m[2][2];
	float root;

	if (trace > 0.0) {
		root = std::sqrt(trace + 1.0f);
		w = 0.5f * root;
		root = 0.5f / root;
		x = (rotation.m[1][2] - rotation.m[2][1]) * root;
		y = (rotation.m[2][0] - rotation.m[0][2]) * root;
		z = (rotation.m[0][1] - rotation.m[1][0]) * root;
	} else {
		size_t s_iNext[3] = { 1, 2, 0 };
		size_t i = 0;
		if (rotation.m[1][1] > rotation.m[0][0])
			i = 1;
		if (rotation.m[2][2] > rotation.m[i][i])
			i = 2;
		size_t j = s_iNext[i];
		size_t k = s_iNext[j];

		root = std::sqrt(rotation.m[i][i] - rotation.m[j][j] - rotation.m[k][k] + 1.0f);
		float* apkQuat[3] = { &x, &y, &z };
		*apkQuat[i] = 0.5f * root;
		root = 0.5f / root;
		w = (rotation.m[j][k] - rotation.m[k][j]) * root;
		*apkQuat[j] = (rotation.m[j][i] + rotation.m[i][j]) * root;
		*apkQuat[k] = (rotation.m[k][i] + rotation.m[i][k]) * root;
	}
}

void Quaternion::FromRotationMatrix(const Matrix4x4& rotation) {
	Matrix3x3 r(rotation);

	FromRotationMatrix(r);
}

void Quaternion::FromAxisAngle(const Vector3& axis, float angle) {
	*this = MathCalculations::MakeRotateAxisAngleQuaternion(axis, angle);
}

Vector3 Quaternion::RotateVector(const Vector3& vector) const {
	return MathCalculations::RotateVector(vector, *this);
}

void Quaternion::LookRotation(const Vector3& forward, const Vector3& up) {
	Math::Vector3 f = forward.Normalize();
	Math::Vector3 r = up.Cross(f).Normalize();
	Math::Vector3 u = f.Cross(r);

	Math::Matrix4x4 m;
	m.m[0][0] = r.x; m.m[0][1] = r.y; m.m[0][2] = r.z;
	m.m[1][0] = u.x; m.m[1][1] = u.y; m.m[1][2] = u.z;
	m.m[2][0] = f.x; m.m[2][1] = f.y; m.m[2][2] = f.z;

	FromRotationMatrix(m);

}

Vector3 Quaternion::xAxis() const {
	float ty = 2.0f * y;
	float tz = 2.0f * z;
	float twy = ty * w;
	float twz = tz * w;
	float txy = ty * x;
	float txz = tz * x;
	float tyy = ty * y;
	float tzz = tz * z;

	return Vector3(1.0f - (tyy + tzz), txy + twz, txz - twy);
}
//-----------------------------------------------------------------------
Vector3 Quaternion::yAxis() const {
	float tx = 2.0f * x;
	float ty = 2.0f * y;
	float tz = 2.0f * z;
	float twx = tx * w;
	float twz = tz * w;
	float txx = tx * x;
	float txy = ty * x;
	float tyz = tz * y;
	float tzz = tz * z;

	return Vector3(txy - twz, 1.0f - (txx + tzz), tyz + twx);
}
//-----------------------------------------------------------------------
Vector3 Quaternion::zAxis() const {
	float tx = 2.0f * x;
	float ty = 2.0f * y;
	float tz = 2.0f * z;
	float twx = tx * w;
	float twy = ty * w;
	float txx = tx * x;
	float txz = tz * x;
	float tyy = ty * y;
	float tyz = tz * y;

	return Vector3(txz + twy, tyz - twx, 1.0f - (txx + tyy));
}

Quaternion Quaternion::Slerp(const Quaternion& q0, const Quaternion& q1, float t) {
	return MathCalculations::Slerp(q0, q1, t);
}

Quaternion operator+(const Quaternion& q1, const Quaternion& q2) {
	return Quaternion(q1.x + q2.x, q1.y + q2.y, q1.z + q2.z, q1.w + q2.w);
}

Quaternion operator-(const Quaternion& q) {
	return Quaternion(-q.x, -q.y, -q.z, -q.w);
}

Quaternion operator*(const Quaternion& q1, const Quaternion& q2) {
	return MathCalculations::Multiply(q1, q2);
}

Quaternion operator*(const Quaternion& q, float s) {
	return Quaternion(q.x * s, q.y * s, q.z * s, q.w * s);
}

Quaternion operator*(float s, const Quaternion& q) {
	return operator*(q, s);
}
Quaternion operator-(const Quaternion& q0, const Quaternion& q1) {
	return Quaternion(q0.x - q1.x, q0.y - q1.y, q0.z - q1.z, q0.w - q1.w);
}

bool operator==(const Quaternion& q1, const Quaternion& q2) {
	return (q1.x == q2.x && q1.y == q2.y && q1.z == q2.z && q1.w == q2.w);
}

}
}