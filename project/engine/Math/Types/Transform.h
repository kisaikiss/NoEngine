#pragma once
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"
namespace NoEngine {
struct Transform {
public:
	Math::Vector3 scale{ Math::Vector3::UNIT_SCALE };
	Math::Quaternion rotation{ Math::Quaternion::IDENTITY };
	Math::Vector3 translate{ Math::Vector3::ZERO };

	Transform* parent = nullptr;

	Transform() = default;

	Transform(const Math::Vector3& position, const Math::Quaternion& rotation, const Math::Vector3& scale)
		: translate(position), rotation(rotation), scale(scale) {
	}
	Transform(const Math::Vector3& x, const Math::Vector3& y, const Math::Vector3& z, const Math::Vector3& w) {
		Math::Matrix4x4 temp(x, y, z, w);
		scale = temp.GetScale();
		rotation = temp.GetRotation();
		translate = temp.GetTranslate();
	}

	Math::Matrix4x4 MakeAffineMatrix4x4();

	Math::Vector3 GetWorldPosition();
};


}