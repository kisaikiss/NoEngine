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
	bool enableSkinning = false;

	// アニメーション切り替え時のブレンドにかける時間(秒)。0以下にすると即切り替えになる
	float blendDuration = 0.2f;

	// これより下はランタイム用（エディタでは基本触らない想定）
	uint32_t preCurrentAnimation = 0;   // 前フレームのcurrentAnimation。切り替え検知に使う
	uint32_t previousAnimation = 0;     // ブレンド元(切り替え前)のアニメーションIndex
	float previousAnimationTime = 0.f;  // 切り替わった瞬間に凍結した、ブレンド元の再生時刻
	bool isBlending = false;
	float blendTimer = 0.f;
};
}
}