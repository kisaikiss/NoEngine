#pragma once
#include "engine/NoEngine.h"

struct EnemyBulletComponent {
	No::Vector3 velocity = { 0.0f, 0.0f, 0.0f };
	int damage = 1;
	No::Entity owner = No::nullEntity;
};
