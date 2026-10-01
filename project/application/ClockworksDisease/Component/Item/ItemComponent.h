#pragma once
#include "engine/NoEngine.h"

struct PowerItemComponent {
	float riseTime = 0.5f;
	float attractTime = 0.5f;
	float riseSpeed = 1.0f;

	float moveTimer = 0.0f;
	bool risen = false;
};

struct PowerItemGetTag { No::Entity playerEntity = No::INVALID_ENTITY; };

struct BigPowerItemComponent {
	float theta = 0.0f;
	float t = 0.0f;
	float scaleT = 0.0f;
	float yPositionOffset = 0.0f;
	float translateMagnification = 5.0f;
	uint32_t grantPower = 5;
};

struct BigPowerGetTag{};

struct SavePointComponent{
	float rotateTime = 3.0f;
	float rotateTimer = 0.0f;
	float colorMagnification = 5.0f;
};