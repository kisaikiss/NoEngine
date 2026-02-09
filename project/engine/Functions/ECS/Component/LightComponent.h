#pragma once
#include "engine/Math/MathInclude.h"

namespace NoEngine::Component {
struct DirectionalLightComponent {
	Math::Color color;
	Math::Vector3 direction;
	float intensity;
};

struct PointLightComponent {
	Math::Color color;
	float intensity;
	float radius;
	float decay;
};

struct SpotLightComponent {
	Math::Color color;
	float intensity;
	float distance;
	float decay;
	float cosAngle;
	float cosFalloffStart;
};

}