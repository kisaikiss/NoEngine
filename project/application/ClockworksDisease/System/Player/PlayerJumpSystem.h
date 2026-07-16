#pragma once
#include "engine/NoEngine.h"

class PlayerJumpSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;

};

