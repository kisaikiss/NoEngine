#pragma once
#include "engine/NoEngine.h"

enum class BatState {
	GENERATE,
	LIVE,
	DEAD,
};

struct BatComponent {
	BatState state = BatState::GENERATE;
	NoEngine::Vector3 defaultTranslate;
	bool isStarted = false;
	float t = 0.f;
};