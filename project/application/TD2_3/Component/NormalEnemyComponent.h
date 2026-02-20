#pragma once
#include "engine/Math/Types/Vector3.h"
#include"../System/Enemy/EnemyStateManager.h"

struct NormalEnemyComponent
{
	No::Vector3 velocity = { 0.0f, 0.0f, 0.0f };
	float friction = 0.98f;
	int32_t hp = 1;
	std::shared_ptr<EnemyStateManager<NormalEnemyComponent>> stateManager;
	 bool isStarted_ = false;
	 No::Vector3 defaultTranslate_;
};