#pragma once
#include "engine/Math/Types/Vector3.h"
#include "engine/Math/Types/Matrix4x4.h"

struct BackGroundComponent
{
	NoEngine::Matrix4x4 uvTransform;
	float time = 0.0f;
	float timeScale = 1.0f;
	float powerFactor = 0.1f;
	float fadeInner = 0.6f;
	float fadeOuter = 1.0f;
};