#pragma once
#include "BatControlSystem.h"

class BatGreenControlSystem : public BatControlSystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	void LiveUpdate(No::Entity entity, No::Registry& registry, float deltaTime) override;

	void Shoot(No::Registry& registry);
};

