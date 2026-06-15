#pragma once
#include "engine/Assets/Skeleton.h"
#include "engine/Assets/Animation.h"
#include "engine/Assets/AssetHandles.h"

namespace NoEngine {

namespace Component {
struct AnimatorComponent {
	std::vector<Asset::AnimationHandle> animationHandles;
	Transform local;
	uint32_t currentAnimation = 0;
	Asset::SkeletonHandle skeletonHandle;
	float time = 0.f;
	bool drawSkeleton = false;
};
}
}