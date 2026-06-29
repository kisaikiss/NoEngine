#pragma once
#include "engine/NoEngine.h"

class GameScene : public No::IScene {
public:
	void Setup() override;
private:
	void AddSystems();

};