#pragma once
#include "engine/NoEngine.h"
#include "engine/Editor/ReflectionMacros.h"

struct PlayerAttackComponent {
	No::Vector2 spawnOffset{ 0.0f, -80.0f };
	No::Vector2 attackSize{ 140.0f, 140.0f };
	float visibleTime = 0.35f;
	int attackLayer = 30;
};
