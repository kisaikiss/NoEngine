#pragma once
#include "engine/NoEngine.h"

struct PlayerComponent {
	float moveSpeed = 10.f;
	float jumpSpeed = 16.f;
	float gravity = -9.8f;
	float yVelocity = 0.f;
	bool infinityJump = false;
};