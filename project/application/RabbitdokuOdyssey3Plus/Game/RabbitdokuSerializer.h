#pragma once
#include "engine/NoEngine.h"

struct SaveData {
	No::Vector2 respawnPoint = No::Vector2::ZERO;
	uint32_t death = 0;
	uint32_t totalDeath = 0;
};

class RabbitdokuSerializer {
public:
	static void SetSaveData(uint32_t saveDataNum);
	static void GameSave(No::Registry& registry, const No::Vector2& respawnPoint, uint32_t deathCount, uint32_t totalDeath);
	static SaveData GameLoad(No::Registry& registry);
};

