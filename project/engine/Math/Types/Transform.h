#pragma once
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"
namespace NoEngine {
struct Transform {
public:
	Vector3 scale{ Vector3::UNIT_SCALE };
	Quaternion rotation{ Quaternion::IDENTITY };
	Vector3 transition{ Vector3::ZERO };

	Transform* parent = nullptr;

	Transform() = default;

	Transform(const Vector3& position, const Quaternion& rotation, const Vector3& scale)
		: transition(position), rotation(rotation), scale(scale) {
	}
	Transform(const Vector3& x, const Vector3& y, const Vector3& z, const Vector3& w) {
		Matrix4x4 temp(x, y, z, w);
		scale = temp.GetScale();
		rotation = temp.GetRotation();
		transition = temp.GetTranslate();
	}

	Matrix4x4 MakeAffineMatrix4x4();
};


}