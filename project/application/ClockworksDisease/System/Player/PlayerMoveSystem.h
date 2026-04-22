#pragma once
#include "engine/NoEngine.h"

class PlayerMoveSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;

};

