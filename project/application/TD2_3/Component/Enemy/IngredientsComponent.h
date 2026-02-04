#pragma once

#include "engine/NoEngine.h"

struct IngredientsComponent {
	static inline const float skGravity = -9.f;
	NoEngine::Vector3 velocity{};
	float t = 0.f;
};