#pragma once
#include "engine/NoEngine.h"

struct EnemyRewardOrbComponent {
	No::Vector2 start = { 0.0f, 0.0f };
	No::Vector2 control = { 0.0f, 0.0f };
	No::Vector2 end = { 0.0f, 0.0f };
	float duration = 0.8f;
	float elapsed = 0.0f;
	int attackPower = 1;
};
