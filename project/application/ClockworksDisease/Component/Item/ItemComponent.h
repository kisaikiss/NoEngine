#pragma once
#include "engine/NoEngine.h"

struct BadgeComponent {
	uint32_t id = 0;
};

struct BigBadgeComponent {
	float theta = 0.0f;
	float t = 0.0f;
	float scaleT = 0.0f;
	float yPositionOffset = 0.0f;
	float translateMagnification = 5.0f;
};

struct BigBadgeGetTag{};