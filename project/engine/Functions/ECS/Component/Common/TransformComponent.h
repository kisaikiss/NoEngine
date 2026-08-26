#pragma once
#include "engine/Math/Types/Transform.h"
#include "engine/Editor/ReflectionMacros.h"

namespace NoEngine {
namespace Component {
struct TransformComponent : public Transform {
	TransformComponent() = default;

	TransformComponent(const Math::Vector3& position, const Math::Quaternion& rotation, const Math::Vector3& scale) : Transform(position, rotation, scale) {}
	Math::Matrix4x4 MakeAffineMatrix4x4(ECS::Registry& registry) const override;
	Math::Vector3 GetWorldScale(ECS::Registry& registry) const;
	Math::Quaternion GetWorldRotation(ECS::Registry& registry) const;
};
}
}