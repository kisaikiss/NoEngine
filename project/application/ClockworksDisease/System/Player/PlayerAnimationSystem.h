#pragma once
#include "engine/NoEngine.h"
class PlayerAnimationSystem: public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:

};

