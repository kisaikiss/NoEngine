#pragma once

#include "engine/Math/Types/Vector3.h"
#include "application/TD2_3/Component/Enemy/BatComponent.h"

enum class PotBossState {
	GENERATE,
	MOVE,
	DEAD,
};

struct PotBossComponent {
	No::Vector3 velocity = { 0.0f, 0.0f, 0.0f };
	PotBossState state = PotBossState::GENERATE;
	int32_t hp = 25;
	float t = 0.f;
	float deadTimer = 0.f;
	float invincibleTimer = 0.f;
	float shootTimer = 0.f;
	bool isStarted = false;
};