#pragma once
#include "engine/Math/MathInclude.h"

namespace NoEngine {
namespace Component {
struct VelocityComponent {
	Math::Vector3 linear;
};

struct Velocity2DComponent {
	Math::Vector2 linear;
};
}
}