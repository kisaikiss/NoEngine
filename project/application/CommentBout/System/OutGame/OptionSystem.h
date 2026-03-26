#pragma once
#include "engine/NoEngine.h"

class OptionSystem : public No::ISystem {
public:
	OptionSystem() { SetStopInPause(false); }
	void Update(No::Registry& registry, float deltaTime) override;
};
