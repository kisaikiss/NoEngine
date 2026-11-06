#pragma once
#include "engine/NoEngine.h"

struct WanderHopEnemyComponent {
	float walkSpeed = 1.4f;
	float chaseSpeed = 3.5f;
	float wanderRadius = 2.0f;
	float detectionDistance = 5.0f;
	float loseDistance = 8.0f;
	float maxHeightDifference = 2.0f;
	float jumpSpeed = 5.0f;
};

struct WanderHopEnemyStateComponent {
	enum class Phase { Wander, AlertHop, Chase };

	Phase phase = Phase::Wander;
	No::Vector3 spawnPosition = No::Vector3::ZERO;
	No::Vector3 wanderTarget = No::Vector3::ZERO;
	float wanderTime = 0.0f;
	bool initialized = false;
	bool justJumped = false;
};
