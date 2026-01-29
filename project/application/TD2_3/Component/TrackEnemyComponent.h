#pragma once

#include "engine/Math/Types/Vector3.h"
#include"../System/Enemy/EnemyStateManager.h"

struct TrackEnemyComponent
{
	NoEngine::Vector3 velocity = { 0.0f, 0.0f, 0.0f };
	float friction = 0.98f;
	int32_t hp = 3;
	std::shared_ptr<EnemyStateManager<TrackEnemyComponent>> stateManager;
	bool isStarted_ = false;
};