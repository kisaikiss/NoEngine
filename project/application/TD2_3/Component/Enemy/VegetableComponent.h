#pragma once
#include "engine/NoEngine.h"

enum class VegetableState {
	GENERATE,
	LIVE,
	DEAD,
};

enum class VegetableShootState {
	NONE,
	STANBY,
	SHOOT,
};

struct VegetableComponent {
	VegetableState state = VegetableState::GENERATE;
	VegetableShootState shootState = VegetableShootState::NONE;
	No::Vector3 defaultTranslate;
	bool isStarted = false;
	float t = 0.f;
	float shootTimer = 0.f;
};
