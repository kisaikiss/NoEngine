#pragma once
namespace NoEngine {

struct Quaternion {
	float x;
	float y;
	float z;
	float w;
};

Quaternion operator+(const Quaternion& q1, const Quaternion& q2);

Quaternion operator-(const Quaternion& q);

Quaternion operator*(const Quaternion& q1, const Quaternion& q2);

Quaternion operator*(const Quaternion& q, float s);

Quaternion operator*(float s, const Quaternion& q);
}