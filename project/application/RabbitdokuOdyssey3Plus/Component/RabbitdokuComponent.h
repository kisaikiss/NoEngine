#pragma once
#include "engine/NoEngine.h"

enum class RabbitdokuState {
	Wait,
	Walk,
	Jump,
	Fall,
	Wall,
	Dead,

	Unknown
};

enum class RabbitdokuDirection {
	kRight,
	kLeft,
};

struct Rabbitdoku {
	float moveSpeed = 500.f;
	float yVelocity = 0.f;
	float gravity = 9.8f;
	float maxFallSpeed = 700.f;

	float jumpSpeed;
	float doubleJumpSpeed;

	float wallJumpTimer = 0.f;
	RabbitdokuDirection wallJumpDirection = RabbitdokuDirection::kRight;
	bool isWallJump = false;

	float deadTimer = 0.0f;

	bool canDoubleJump = true;
	bool sizeCollide = false;
	RabbitdokuState nextState = RabbitdokuState::Unknown;
	RabbitdokuState state = RabbitdokuState::Wait;
};