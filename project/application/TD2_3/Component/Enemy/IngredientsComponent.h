#pragma once

#include "engine/NoEngine.h"

struct IngredientsComponent {
	static inline const float skGravity = -9.f;
	No::Vector3 velocity{};
	float t = 0.f;
	bool isCollide = false;
};