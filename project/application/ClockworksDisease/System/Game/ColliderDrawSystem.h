#pragma once
#include "engine/NoEngine.h"

class ColliderDrawSystem : public No::ISystem {
public:
	ColliderDrawSystem() { SetStopInGameStop(false); }
	void Update(No::Registry& registry, float deltaTime) override;
};

