#pragma once
#include "engine/NoEngine.h"

class RabbitdokuSerializer {
public:
	static void SetSaveData(uint32_t saveDataNum);
	static void GameSave(No::Registry& registry, const No::Vector2& respawnPoint);
	static No::Vector2 GameLoad(No::Registry& registry);
};

