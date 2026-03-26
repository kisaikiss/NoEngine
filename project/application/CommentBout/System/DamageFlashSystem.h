#pragma once
#include "engine/NoEngine.h"

class DamageFlashSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
