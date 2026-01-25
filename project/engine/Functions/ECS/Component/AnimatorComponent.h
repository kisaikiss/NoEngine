#pragma once
#include "engine/Assets/Skeleton.h"
#include "engine/Assets/Animation.h"

namespace NoEngine {

namespace Component {
struct AnimatorComponent {
	Animation* animation = nullptr;
	Skeleton* skeleton = nullptr;
	float time = 0.f;
	bool drawSkeleton = false;
};
}
}