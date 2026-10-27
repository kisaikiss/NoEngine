#pragma once
#include "engine/NoEngine.h"

class LevelDoorSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
