#pragma once
#include"engine/Math/Types/Vector3.h"

struct RingAnimationComponent
{ 
	float pressedTime = 0.0f;
	float releaseTime = 0.0f;
	float tTemp = 0.0f;
	static constexpr float kChargeScale = 0.2f;
	static constexpr float pressedSpeed = 2.0f;
};