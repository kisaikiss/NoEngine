#pragma once
#include "engine/Math/Types/Vector3.h"
#include"../System/Enemy/EnemyStateManager.h"

struct NormalEnemyComponent
{
	NoEngine::Vector3 velocity = { 0.0f, 0.0f, 0.0f };
	float friction = 0.98f;
	uint32_t hp = 1;
	No::Entity entity = 0;
	std::shared_ptr<EnemyStateManager<NormalEnemyComponent>> stateManager;
	 bool isStarted_ = false;
	 NoEngine::Vector3 defaultTranslate_;
};