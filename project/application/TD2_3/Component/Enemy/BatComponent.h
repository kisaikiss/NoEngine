#pragma once
#include "engine/NoEngine.h"

enum class BatState {
	GENERATE,
	LIVE,
	DEAD,
};

enum class BatShootState {
	NONE,
	STANBY,
	SHOOT,
};

struct BatComponent {
	BatState state = BatState::GENERATE;
	BatShootState shootState = BatShootState::NONE;
	No::Vector3 defaultTranslate;
	bool isStarted = false;
	float t = 0.f;
	float shootTimer = 0.f;
};