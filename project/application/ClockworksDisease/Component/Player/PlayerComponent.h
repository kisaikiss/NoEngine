#pragma once
#include "engine/NoEngine.h"

struct PlayerComponent {
	No::Vector3 velocity;
	float moveSpeed = 1.f;
};