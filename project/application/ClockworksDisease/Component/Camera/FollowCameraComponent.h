#pragma once
#include "engine/NoEngine.h"

struct FollowCameraComponent {
	float maxDistance = 15.0f;
	float distance = 15.0f;
	float theta = 0.f;
	float phi = 1.f;

	float moveSpeed = 2.0f;
	
	float minPhi = PI / 5.0f;
	float maxPhi = PI / 2.0f;

	float minFov = 0.65f;
	float maxFov = 0.85f;
	float playerPosOffset = 2.0f;
};