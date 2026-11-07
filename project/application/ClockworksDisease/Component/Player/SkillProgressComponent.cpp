#include "SkillProgressComponent.h"
#include "PlayerMoveTags.h"

REFLECT_STRUCT_BEGIN(SkillProgressComponent, "Application")
REFLECT_FIELD(skillPoints),
REFLECT_ARRAY_FIELD(purchasedSkillIds)
REFLECT_STRUCT_END(SkillProgressComponent)

bool PlayerSkills::HasPurchased(const SkillProgressComponent& progress, std::string_view skillId) {
	return std::ranges::find(progress.purchasedSkillIds, skillId) != progress.purchasedSkillIds.end();
}

bool PlayerSkills::HasPurchasedAirDash(No::Registry& registry, No::Entity player) {
	const auto* progress = registry.GetComponent<SkillProgressComponent>(player);
	return progress && HasPurchased(*progress, AirDashId);
}

void PlayerSkills::ApplyPurchasedAbilities(No::Registry& registry, No::Entity player) {
	if (HasPurchasedAirDash(registry, player) && !registry.Has<AirDashTag>(player)) {
		registry.AddComponent<AirDashTag>(player);
	}
}
