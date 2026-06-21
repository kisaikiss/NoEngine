#pragma once
#include "engine/NoEngine.h"

struct BadgeComponent {
	uint32_t id = 0;
};

struct BigBadgeComponent {
	No::TransformComponent* playerTransform = nullptr;
};

struct BigBadgeGetTag{};