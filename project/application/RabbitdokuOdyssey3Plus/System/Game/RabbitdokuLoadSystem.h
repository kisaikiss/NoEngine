#pragma once
#include "engine/NoEngine.h"

class RabbitdokuLoadSystem : public No::ISystem {
public:
	RabbitdokuLoadSystem(){ SetStopInGameStop(false); SetStopInPause(false); }
	void Update(No::Registry& registry, float deltaTime) override;
private:
	bool isStartFrame_ = true;
};

