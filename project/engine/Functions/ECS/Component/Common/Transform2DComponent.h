#pragma once
#include "engine/Math/Types/Transform2D.h"

namespace NoEngine {
namespace Component {
struct Transform2DComponent : public Transform2D {
	Transform2DComponent() = default;
	Transform2DComponent(const Math::Vector2& position, float rotation, const Math::Vector2& scale) : Transform2D(position, rotation, scale) {}

	Math::Matrix3x3 MakeAffineMatrix3x3(ECS::Registry& registry) const override;
	Math::Matrix4x4 MakeAffineMatrix4x4(ECS::Registry& registry) const override;
	Math::Vector2 GetWorldScale(ECS::Registry& registry) const;
};
}
}