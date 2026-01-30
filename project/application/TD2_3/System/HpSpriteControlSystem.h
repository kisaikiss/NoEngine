#pragma once

#include "engine/NoEngine.h"

class HpSpriteControlSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	float angle_ = 0.f;
};

