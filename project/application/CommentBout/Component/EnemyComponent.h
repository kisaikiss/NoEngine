#pragma once
#include "engine/NoEngine.h"

struct EnemyComponent {
	int hp = 10;
	int maxHp = 10;
	float moveSpeed = 3.0f;
	No::Vector3 moveDirection = { 0.0f, 0.0f, -1.0f };
	int groupId = 0;
	bool wasCollidingWithAttack = false;
};
