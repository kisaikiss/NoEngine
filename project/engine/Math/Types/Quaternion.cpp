#include "Quaternion.h"
#include "Calculations/QuaternionCalculations.h"

#include "Matrix4x4.h"

namespace NoEngine {
const Quaternion Quaternion::ZERO(0, 0, 0, 0);
const Quaternion Quaternion::IDENTITY(0, 0, 0, 1);

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
}