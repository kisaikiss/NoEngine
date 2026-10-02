#pragma once
#include "engine/NoEngine.h"

class WarpBlockSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};

