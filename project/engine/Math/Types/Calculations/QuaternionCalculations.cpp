#include "QuaternionCalculations.h"
#include "../Vector3.h"
#include "Vector3Calculations.h"
#include "../Matrix4x4.h"

namespace NoEngine {
namespace MathCalculations {
Quaternion IdentityQuaternion() {
	return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
}

Quaternion Conjugate(const Quaternion& quaternion) {
	return Quaternion(-quaternion.x, -quaternion.y, -quaternion.z, quaternion.w);
}

Quaternion Multiply(const Quaternion& q1, const Quaternion& q2) {
	Quaternion result{};
	result.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
	result.x = q1.y * q2.z - q1.z * q2.y + q2.w * q1.x + q1.w * q2.x;
	result.y = q1.z * q2.x - q1.x * q2.z + q2.w * q1.y + q1.w * q2.y;
	result.z = q1.x * q2.y - q1.y * q2.x + q2.w * q1.z + q1.w * q2.z;

	return result;
}

float Dot(const Quaternion& q1, const Quaternion& q2) {
	return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
}

Quaternion Normalize(const Quaternion& q) {
	float norm = Norm(q);
	assert(norm != 0);
	return Quaternion(q.x / norm, q.y / norm, q.z / norm, q.w / norm);
}

float Norm(const Quaternion& q) {

	float result = std::sqrtf(std::powf(q.x, 2.0f) + std::powf(q.y, 2.0f) + std::powf(q.z, 2.0f) + std::powf(q.w, 2.0f));

	return result;
}

Quaternion Inverse(const Quaternion& q) {
	float squaredNorm = std::powf(Norm(q), 2.0f);
	Quaternion conjugate = Conjugate(q);

	return Quaternion(conjugate.x / squaredNorm, conjugate.y / squaredNorm, conjugate.z / squaredNorm, conjugate.w / squaredNorm);
}

Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle) {
	Vector3 n = Normalize(axis);
	Quaternion result{};
	result.w = std::cos(angle / 2.0f);
	result.x = n.x * std::sin(angle / 2.0f);
	result.y = n.y * std::sin(angle / 2.0f);
	result.z = n.z * std::sin(angle / 2.0f);

	result = Normalize(result);

	return result;
}

Vector3 RotateVector(const Vector3& vector, const Quaternion& quaternion) {
	Quaternion r{};
	r.x = vector.x;
	r.y = vector.y;
	r.z = vector.z;

	Quaternion q = Normalize(quaternion);

	Quaternion result = q * r * Conjugate(q);

	return Vector3(result.x, result.y, result.z);
}

Matrix4x4 MakeRotateMatrix(const Quaternion& quaternion) {
	Quaternion q = Normalize(quaternion);

	float xx = q.x * q.x;
	float yy = q.y * q.y;
	float zz = q.z * q.z;
	float ww = q.w * q.w;
	float xy = q.x * q.y;
	float xz = q.x * q.z;
	float yz = q.y * q.z;
	float wx = q.w * q.x;
	float wy = q.w * q.y;
	float wz = q.w * q.z;


	Matrix4x4 result = {
		{ww + xx - yy - zz, 2.0f * (xy + wz), 2.0f * (xz - wy),0.0f,
		 2.0f * (xy - wz), ww - xx + yy - zz, 2.0f * (yz + wx), 0.0f,
		 2.0f * (xz + wy), 2.0f * (yz - wx),ww - xx - yy + zz,0.0f,
		0.0f,0.0f,0.0f,1.0f}
	};

	return result;
}

Quaternion Slerp(Quaternion q0, Quaternion q1, float t) {
	float dot = Dot(q0, q1);
	if (dot < 0) {
		q0 = -q0;
		dot = -dot;
	}

	float theta = std::acos(dot);

	float scale0 = std::sin((1 - t) * theta) / std::sin(theta);
	float scale1 = std::sin(t * theta) / std::sin(theta);

	return scale0 * q0 + scale1 * q1;
}
}
}