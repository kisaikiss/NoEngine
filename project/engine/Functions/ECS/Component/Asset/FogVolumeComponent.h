#pragma once
#include "engine/Math/Types/Vector3.h"

namespace NoEngine::Component {

struct FogVolumeComponent {
	bool enabled = true;
	float density = 0.35f;
	// 各辺の長さに対する、境界から内側へのフェード幅。
	float edgeFade = 0.15f;
	float noiseFrequency = 2.5f;
	float noiseStrength = 0.f;
	// Box の各辺の長さを 1 とした、1 秒あたりの移動量。
	Math::Vector3 windVelocity{};
};

}
