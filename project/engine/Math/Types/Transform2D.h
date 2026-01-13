#pragma once
#include "Vector2.h"
#include "Matrix3x3.h"

namespace NoEngine {
struct Transform2D {
	Vector2 scale{ Vector2::UNIT_SCALE };
	float rotation{};
	Vector2 translate{ Vector2::ZERO };

	Transform2D* parent = nullptr;

	Transform2D() = default;

	Transform2D(const Vector2& position, float rotation, const Vector2& scale)
		: translate(position), rotation(rotation), scale(scale) {
	}

	Matrix3x3 MakeAffineMatrix3x3() const;
};
}