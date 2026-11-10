#pragma once

namespace NoEngine::Component {

struct FogVolumeComponent {
	bool enabled = true;
	float density = 0.35f;
	// 各辺の長さに対する、境界から内側へのフェード幅。
	float edgeFade = 0.15f;
};

}
