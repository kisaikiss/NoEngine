#pragma once
#include "engine/Math/MathInclude.h"

namespace NoEngine::Component {
struct DirectionalLightComponent {
	Color color;
	Vector3 direction;
	float intensity;
};

struct PointLightComponent {
	Color color;
	float intensity;
	float radius;
	float decay;
};

struct SpotLightComponent {
	Color color;
	float intensity;
	float distance;
	float decay;
	float cosAngle;
	float cosFalloffStart;
};

}