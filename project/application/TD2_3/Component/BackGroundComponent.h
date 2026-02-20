#pragma once
#include "engine/Math/Types/Matrix4x4.h"
#include "engine/Math/Color/Color.h"

struct BackGroundComponent
{
	No::Matrix4x4 uvTransform;
	float time = 0.0f;
	float timeScale = 1.0f;
	float powerFactor = 0.1f;
	float fadeInner = 0.75f;
	float fadeOuter = 1.1f;
	float seed = 123;
	float variant = 0.0f;
	uint32_t useRing = 0;
	No::Color bgColor = No::Color(0.01f, 0.26f, 0.42f, 1.0f);
	No::Color ringColor = No::Color(0.01f, 0.36f, 0.57f, 1.0f);
};