#pragma once
#include "engine/NoEngine.h"

class PlayerLevelUpSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	void EnhancementsUponLevelingUp(No::Registry& registry, No::Entity e, uint32_t level);
};