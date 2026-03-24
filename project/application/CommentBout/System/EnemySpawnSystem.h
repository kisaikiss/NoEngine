#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// SpawnEnemyRequest を消費して敵を生成するSystem。
/// </summary>
class EnemySpawnSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
