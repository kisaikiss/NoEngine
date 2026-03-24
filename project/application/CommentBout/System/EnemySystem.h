#pragma once
#include "engine/NoEngine.h"

struct RailEnemySpawnEventParams;

void SpawnRailEnemies(No::Registry& registry, const RailEnemySpawnEventParams& params);

class EnemySystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
