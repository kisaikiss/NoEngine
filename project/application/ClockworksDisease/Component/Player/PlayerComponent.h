#pragma once
#include "engine/NoEngine.h"

enum class PlayerState {
	kWait,
	kWalk,
	kJump,
	kMultiJump,
	kHighJump,
	kFall
};

struct PlayerComponent {
	float moveSpeed = 10.f;
	float doubleJumpSpeed = 4.f;
	float jumpSpeed = 16.f;
	float highJumpSpeed = 17.f;
	float gravity = -9.8f;
	float yVelocity = 0.f;
	float stamina = 0.0f;
	float maxStamina = 0.0f;
 	No::Vector3 groundNormal = No::Vector3::UP;
	bool infinityJump = false;
	PlayerState state = PlayerState::kWait;
};

struct LevelComponent {
	uint32_t power = 0;
	uint32_t nowLevel = 1;
	uint32_t nextLevelUp = 30;
};