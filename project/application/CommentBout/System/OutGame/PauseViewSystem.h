#pragma once
#include "engine/NoEngine.h"

class PauseViewSystem : public No::ISystem {
public:
	PauseViewSystem() { SetStopInPause(false); }
	void Update(No::Registry& registry, float deltaTime) override;
};
