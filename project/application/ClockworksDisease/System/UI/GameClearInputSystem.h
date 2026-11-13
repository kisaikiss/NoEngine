#pragma once
#include "engine/NoEngine.h"

class GameClearInputSystem : public No::ISystem {
public:
	GameClearInputSystem() { SetStopInGameStop(false); SetStopInPause(false); }
	void Update(No::Registry& registry, float deltaTime) override;
};
