#pragma once
#include "engine/NoEngine.h"

enum class RabbitdokuState {
	Wait,
	Walk,

	Unknown
};

struct Rabbitdoku {
	No::Vector2 respawnPoint = No::Vector2::ZERO;
	float moveSpeed = 500.f;
	float yVelocity = 0.f;
	float jumpSpeed;
	float doubleJumpSpeed;
	bool canDoubleJump = true;
	RabbitdokuState nextState = RabbitdokuState::Unknown;
	RabbitdokuState state = RabbitdokuState::Wait;
};