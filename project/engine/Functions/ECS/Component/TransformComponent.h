#pragma once
#include "engine/Math/Types/Transform.h"

namespace NoEngine {
namespace Component {
struct TransformComponent : public Transform {
	TransformComponent() = default;

	TransformComponent(const Vector3& position, const Quaternion& rotation, const Vector3& scale) : Transform(position, rotation, scale) {}
};
}
}