#pragma once
#include "engine/Assets/Skeleton.h"
#include "engine/Assets/Animation.h"

namespace NoEngine {

namespace Component {
struct AnimatorComponent {
	std::span<Animation> animation;
	uint32_t currentAnimation = 0;
	Skeleton* skeleton = nullptr;
	float time = 0.f;
	bool drawSkeleton = false;
};
}
}