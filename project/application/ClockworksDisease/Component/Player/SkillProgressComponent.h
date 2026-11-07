#pragma once
#include "engine/NoEngine.h"
#include "PlayerComponent.h"

struct SkillProgressComponent {
	uint32_t skillPoints = 0;
	std::vector<std::string> purchasedSkillIds;
};

namespace PlayerSkills {
struct SkillDefinition {
	std::string id;
	std::string name;
	uint32_t cost = 0;
	std::string iconTexture;
	std::string previewTexture;
	std::string description;
	PlayerAbility ability = PlayerAbility::kNone;
	float staminaIncrease = 0.f;
	std::string requiredStage;
	std::vector<std::string> prerequisites;
};

struct SkillGroup {
	std::string name;
	std::vector<SkillDefinition> skills;
};

enum class PurchaseStatus { kAvailable, kPurchased, kStageLocked, kPrerequisiteLocked, kInsufficientPoints };

const std::vector<SkillGroup>& GetCatalog();
const SkillDefinition* FindSkill(std::string_view id);
const SkillDefinition* FindMissingPrerequisite(const SkillProgressComponent& progress, const SkillDefinition& skill);
PurchaseStatus GetPurchaseStatus(const SkillProgressComponent& progress, const SkillDefinition& skill);
bool TryPurchase(No::Registry& registry, No::Entity player, std::string_view skillId);
bool HasPurchasedAbility(No::Registry& registry, No::Entity player, PlayerAbility ability);
void ApplyPurchasedAbilities(No::Registry& registry, No::Entity player);
}
