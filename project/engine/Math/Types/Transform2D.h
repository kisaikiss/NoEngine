#pragma once
#include "Vector2.h"
#include "Matrix3x3.h"

namespace NoEngine {
struct Transform2D {
	Math::Vector2 scale{ Math::Vector2::UNIT_SCALE };
	float rotation{};
	Math::Vector2 translate{ Math::Vector2::ZERO };

	Transform2D* parent = nullptr;

	Transform2D() = default;

	Transform2D(const Math::Vector2& position, float rotation, const Math::Vector2& scale)
		: translate(position), rotation(rotation), scale(scale) {
	}

	Math::Matrix3x3 MakeAffineMatrix3x3() const;
};
}