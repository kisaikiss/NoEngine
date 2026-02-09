#pragma once
#include "engine/NoEngine.h"

struct EnemyBulletComponent {
	No::Vector3 velocity{};
	bool isCollide = false;
	float t = 0.f;
};