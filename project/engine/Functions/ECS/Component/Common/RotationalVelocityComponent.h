#pragma once
#include "engine/Math/MathInclude.h"

namespace NoEngine {
namespace Component {
struct RotationalVelocityComponent {
	Math::Vector3 linear;
};

struct RotationalVelocity2DComponent {
	float linear;
};
}
}