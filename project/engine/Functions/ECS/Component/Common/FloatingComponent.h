#pragma once

#include "engine/Math/MathInclude.h"

namespace NoEngine {
namespace Component {
struct FloatingComponent {
	Math::Vector3 velocity;
	Math::Vector3 amplitude;

	// 実行時の状態は、前フレームで加えた浮遊オフセット。
	Math::Vector3 previousOffset{ Math::Vector3::ZERO };
	float elapsedTime = 0.0f;
};

struct Floating2DComponent {
	Math::Vector2 velocity;
	Math::Vector2 amplitude;


	// 実行時の状態は、前フレームで加えた浮遊オフセット。
	Math::Vector2 previousOffset{ Math::Vector2::ZERO };
	float elapsedTime = 0.0f;
};
}
}
