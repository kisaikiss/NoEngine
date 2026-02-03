#pragma once

#include "engine/Math/Types/Vector3.h"
#include "application/TD2_3/Component/Enemy/BatComponent.h"

enum class BatBossState {
	GENERATE,
	MOVE,
	DEAD,
};

struct BatBossComponent
{
	NoEngine::Vector3 velocity = { 0.0f, 0.0f, 0.0f };
	BatBossState state = BatBossState::GENERATE;
	BatShootState shootState = BatShootState::NONE;
	int32_t hp = 10;
	float t = 0.f;
	float deadTimer = 0.f;
	float invincibleTimer = 0.f;
	float shootTimer = 0.f;
	bool isStarted = false;
};