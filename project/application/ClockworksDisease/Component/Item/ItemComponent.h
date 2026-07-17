#pragma once
#include "engine/NoEngine.h"

struct PowerItemComponent {};

struct BigPowerItemComponent {
	float theta = 0.0f;
	float t = 0.0f;
	float scaleT = 0.0f;
	float yPositionOffset = 0.0f;
	float translateMagnification = 5.0f;
	uint32_t grantPower = 5;
};

struct BigPowerGetTag{};