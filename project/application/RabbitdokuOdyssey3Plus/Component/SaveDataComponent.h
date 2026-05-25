#pragma once
#include "engine/NoEngine.h"

struct SaveDataComponent {
	No::Vector2 respawnPoint = No::Vector2::ZERO;
	uint32_t deathCount = 0;
};