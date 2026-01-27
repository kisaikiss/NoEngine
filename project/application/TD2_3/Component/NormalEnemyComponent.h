#pragma once
#include "engine/Math/Types/Vector3.h"

struct NormalEnemyComponent
{
	NoEngine::Vector3 velocity = { 0.0f, 0.0f, 0.0f };
	float friction = 0.98f;
	uint32_t hp = 1;
};