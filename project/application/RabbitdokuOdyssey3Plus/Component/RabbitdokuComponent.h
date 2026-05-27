#pragma once
#include "engine/NoEngine.h"

enum class RabbitdokuState {
	Wait,
	Walk,
	Jump,
	Wall,

	Unknown
};

enum class RabbitdokuDirection {
	kRight,
	kLeft,
};

struct Rabbitdoku {
	float moveSpeed = 500.f;
	float yVelocity = 0.f;

	float jumpSpeed;
	float doubleJumpSpeed;

	float wallJumpTimer = 0.f;
	RabbitdokuDirection wallJumpDirection = RabbitdokuDirection::kRight;
	bool isWallJump = false;

	bool canDoubleJump = true;
	bool sizeCollide = false;
	RabbitdokuState nextState = RabbitdokuState::Unknown;
	RabbitdokuState state = RabbitdokuState::Wait;
};