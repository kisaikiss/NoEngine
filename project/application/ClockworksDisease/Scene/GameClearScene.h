#pragma once
#include "engine/NoEngine.h"

class GameClearScene : public No::IScene {
public:
	void Setup() override;
private:
	void NotSystemUpdate() override;
	void AddSystems();
};