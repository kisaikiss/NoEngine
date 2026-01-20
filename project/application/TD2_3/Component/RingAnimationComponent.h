#pragma once
#include"engine/Math/Types/Vector3.h"

struct RingAnimationComponent
{ 
	float pressedTime = 0.0f;
	float releaseTime = 0.0f;
	NoEngine::Vector3 baseScale = NoEngine::Vector3::UNIT_SCALE;
	NoEngine::Vector3 targetScale = NoEngine::Vector3::UNIT_SCALE;
	NoEngine::Vector3 currentScale = NoEngine::Vector3::UNIT_SCALE;

};