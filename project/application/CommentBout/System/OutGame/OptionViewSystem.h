#pragma once
#include "engine/NoEngine.h"

class OptionViewSystem : public No::ISystem {
public:
	OptionViewSystem() { SetStopInPause(false); }
	void Update(No::Registry& registry, float deltaTime) override;
};
