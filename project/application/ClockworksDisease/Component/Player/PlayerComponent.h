#pragma once
#include "engine/NoEngine.h"

struct PlayerComponent {
	float moveSpeed = 10.f;
	float doubleJumpSpeed = 4.f;
	float jumpSpeed = 16.f;
	float gravity = -9.8f;
	float yVelocity = 0.f;
	float stamina = 0.0f;
	float maxStamina = 0.0f;
 	No::Vector3 groundNormal = No::Vector3::UP;
	bool infinityJump = false;
	uint32_t power = 0;
};