#pragma once
#include "engine/NoEngine.h"

struct SkillProgressComponent {
	uint32_t skillPoints = 0;
	std::vector<std::string> purchasedSkillIds;
};

namespace PlayerSkills {
inline constexpr char AirDashId[] = "air_dash";

bool HasPurchased(const SkillProgressComponent& progress, std::string_view skillId);
bool HasPurchasedAirDash(No::Registry& registry, No::Entity player);
void ApplyPurchasedAbilities(No::Registry& registry, No::Entity player);
}
