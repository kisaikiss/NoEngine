#pragma once
#include "engine/NoEngine.h"
#include <vector>

struct AttackDamageComponent {
	int damage = 10;
	std::vector<No::Entity> hitEnemies;
	bool isFirstFrameConsumed = false;
};
