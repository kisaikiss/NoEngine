#pragma once
#include "engine/Assets/AssetHandles.h"
#include "engine/Math/Types/Transform.h"

namespace NoEngine {

namespace Component {
struct AnimatorComponent {
	std::vector<Asset::AnimationHandle> animationHandles;
	Transform local;
	uint32_t currentAnimation = 0;
	Asset::SkeletonHandle skeletonHandle;
	float time = 0.f;
	float animationSpeedMagnification = 1.0f;
	bool drawSkeleton = false;
};
}
}