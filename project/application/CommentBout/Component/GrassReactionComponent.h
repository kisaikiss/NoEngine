#pragma once
#include "engine/NoEngine.h"

struct GrassReactionComponent {
	bool wasColliding = false;
	No::Vector2 effectOffset{ 60.0f, -60.0f };
	No::Vector2 effectSize{ 60.0f, 60.0f };
	float effectLifetime = 0.3f;
	int effectLayer = 40;
};
