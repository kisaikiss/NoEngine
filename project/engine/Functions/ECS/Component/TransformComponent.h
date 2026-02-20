#pragma once
#include "engine/Math/Types/Transform.h"
#include "engine/Editor/ReflectionMacros.h"

namespace NoEngine {
namespace Component {
struct TransformComponent : public Transform {
	TransformComponent() = default;

	TransformComponent(const Math::Vector3& position, const Math::Quaternion& rotation, const Math::Vector3& scale) : Transform(position, rotation, scale) {}

};
}
}