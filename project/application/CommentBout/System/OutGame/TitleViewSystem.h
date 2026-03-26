#pragma once
#include "engine/NoEngine.h"

class TitleViewSystem : public No::ISystem {
public:
	TitleViewSystem() { SetStopInPause(false); }
	void Update(No::Registry& registry, float deltaTime) override;
};
