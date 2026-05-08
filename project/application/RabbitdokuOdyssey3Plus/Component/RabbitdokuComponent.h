#pragma once
#include "engine/NoEngine.h"

struct Rabbitdoku {
	No::Vector2 respawnPoint = No::Vector2::ZERO;
	float yVelocity = 0.f;
	float jumpSpeed;
};