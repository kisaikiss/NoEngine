#pragma once
#include "engine/NoEngine.h"

class SpringSystem : public No::ISystem {
public:
	SpringSystem() { SetStopInPause(false); }
	void Update(No::Registry& registry, float deltaTime) override;
};