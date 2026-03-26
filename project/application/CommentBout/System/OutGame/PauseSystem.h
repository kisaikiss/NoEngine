#pragma once
#include "engine/NoEngine.h"

class PauseSystem : public No::ISystem {
public:
	PauseSystem() { SetStopInPause(false); }
	void Update(No::Registry& registry, float deltaTime) override;
};
