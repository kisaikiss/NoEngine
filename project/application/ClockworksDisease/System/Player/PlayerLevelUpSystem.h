#pragma once
#include "engine/NoEngine.h"
#include "../../Component/Player/PlayerComponent.h"

struct LevelUpEffectTag{};

class PlayerLevelUpSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	void EnhancementsUponLevelingUp(No::Registry& registry, No::Entity e, uint32_t level);
	void GrantAbility(No::Registry& registry, No::Entity e, PlayerAbility ability);
};