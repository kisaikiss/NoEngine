#pragma once
#include "engine/NoEngine.h"

class TitleSystem : public No::ISystem {
public:
	TitleSystem() { SetStopInPause(false); }
	void Update(No::Registry& registry, float deltaTime) override;
};
