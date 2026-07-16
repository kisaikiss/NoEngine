#pragma once
#include "engine/NoEngine.h"

class PlayerVerticalVelocitySystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;

};
