#pragma once
#include "Vector2.h"
#include "Matrix3x3.h"
#include "Matrix4x4.h"

#include "engine/Functions/ECS/Registry.h"

namespace NoEngine {
struct Transform2D {
	Math::Vector2 scale{ Math::Vector2::UNIT_SCALE };
	float rotation{};
	Math::Vector2 translate{ Math::Vector2::ZERO };

	ECS::Entity parent = ECS::INVALID_ENTITY;

	Transform2D() = default;

	Transform2D(const Math::Vector2& position, float rotation, const Math::Vector2& scale)
		: translate(position), rotation(rotation), scale(scale) {
	}

	virtual Math::Matrix3x3 MakeAffineMatrix3x3(ECS::Registry& registry) const;
	virtual Math::Matrix4x4 MakeAffineMatrix4x4(ECS::Registry& registry) const;

	Math::Vector2 GetWorldPosition(ECS::Registry& registry) const;
};
}