#include "SkillProgressComponent.h"
#include "PlayerMoveTags.h"
#include "../../System/Game/MainStageProgress.h"

REFLECT_STRUCT_BEGIN(SkillProgressComponent, "Application")
REFLECT_FIELD(skillPoints),
REFLECT_ARRAY_FIELD(purchasedSkillIds)
REFLECT_STRUCT_END(SkillProgressComponent)

namespace {
constexpr char kCatalogPath[] = "resources/game/ClockworksDisease/Skills/SkillCatalog.json";

bool HasPurchased(const SkillProgressComponent& progress, std::string_view skillId) {
	return std::ranges::find(progress.purchasedSkillIds, skillId) != progress.purchasedSkillIds.end();
}

template<class Tag>
void GrantTag(No::Registry& registry, No::Entity player) {
	if (!registry.Has<Tag>(player)) registry.AddComponent<Tag>(player);
}
}

const std::vector<PlayerSkills::SkillGroup>& PlayerSkills::GetCatalog() {
	static const auto catalog = [] {
		std::vector<SkillGroup> groups;
		try {
			std::ifstream file(kCatalogPath);
			const auto data = nlohmann::json::parse(file);
			std::unordered_set<std::string> ids;
			for (const auto& column : data.at("groups")) {
				SkillGroup group{ column.at("name").get<std::string>(), {} };
				for (const auto& entry : column.at("skills")) {
					SkillDefinition skill;
					skill.id = entry.at("id").get<std::string>();
					skill.name = entry.at("name").get<std::string>();
					const int cost = entry.at("cost").get<int>();
					skill.iconTexture = entry.at("iconTexture").get<std::string>();
					skill.previewTexture = entry.at("previewTexture").get<std::string>();
					skill.description = entry.at("description").get<std::string>();
					skill.staminaIncrease = entry.value("staminaIncrease", 0.f);
					skill.requiredStage = entry.value("requiredStage", std::string{});
					skill.prerequisites = entry.value("prerequisites", std::vector<std::string>{});
					const auto ability = magic_enum::enum_cast<PlayerAbility>(entry.value("ability", std::string("kNone")));
					if (!ability || cost < 0 || skill.id.empty() || !std::isfinite(skill.staminaIncrease)
						|| skill.staminaIncrease < 0.f || (*ability == PlayerAbility::kNone && skill.staminaIncrease == 0.f)) {
						throw std::runtime_error("Invalid skill: " + skill.id);
					}
					// 前提を先に定義することで、未定義のIDと循環依存を検出できる。
					for (const auto& id : skill.prerequisites) {
						if (!ids.contains(id)) throw std::runtime_error("Prerequisite must precede " + skill.id + ": " + id);
					}
					if (!ids.insert(skill.id).second) throw std::runtime_error("Duplicate skill: " + skill.id);
					skill.cost = static_cast<uint32_t>(cost);
					skill.ability = *ability;
					group.skills.push_back(std::move(skill));
				}
				if (!group.skills.empty()) groups.push_back(std::move(group));
			}
		} catch (const std::exception& error) {
			LogWarning(std::string("Failed to load skill catalog: ") + error.what());
			groups.clear();
		}
		return groups;
	}();
	return catalog;
}

const PlayerSkills::SkillDefinition* PlayerSkills::FindSkill(std::string_view id) {
	for (const auto& group : GetCatalog()) {
		const auto skill = std::ranges::find(group.skills, id, &SkillDefinition::id);
		if (skill != group.skills.end()) return &*skill;
	}
	return nullptr;
}

const PlayerSkills::SkillDefinition* PlayerSkills::FindMissingPrerequisite(
	const SkillProgressComponent& progress, const SkillDefinition& skill) {
	for (const auto& id : skill.prerequisites) {
		if (!HasPurchased(progress, id)) return FindSkill(id);
	}
	return nullptr;
}

PlayerSkills::PurchaseStatus PlayerSkills::GetPurchaseStatus(const SkillProgressComponent& progress, const SkillDefinition& skill) {
	if (HasPurchased(progress, skill.id)) return PurchaseStatus::kPurchased;
	if (!skill.requiredStage.empty() && !MainStageProgress::HasReachedStage(skill.requiredStage)) return PurchaseStatus::kStageLocked;
	if (FindMissingPrerequisite(progress, skill)) return PurchaseStatus::kPrerequisiteLocked;
	if (progress.skillPoints < skill.cost) return PurchaseStatus::kInsufficientPoints;
	return PurchaseStatus::kAvailable;
}

bool PlayerSkills::TryPurchase(No::Registry& registry, No::Entity player, std::string_view skillId) {
	const auto* skill = FindSkill(skillId);
	auto* progress = registry.GetComponent<SkillProgressComponent>(player);
	auto* stats = registry.GetComponent<PlayerComponent>(player);
	if (!skill || !progress || !stats || GetPurchaseStatus(*progress, *skill) != PurchaseStatus::kAvailable) return false;

	progress->purchasedSkillIds.push_back(skill->id);
	progress->skillPoints -= skill->cost;
	stats->maxStamina += skill->staminaIncrease;
	stats->stamina += skill->staminaIncrease;
	ApplyPurchasedAbilities(registry, player);
	return true;
}

bool PlayerSkills::HasPurchasedAbility(No::Registry& registry, No::Entity player, PlayerAbility ability) {
	const auto* progress = registry.GetComponent<SkillProgressComponent>(player);
	return progress && std::ranges::any_of(progress->purchasedSkillIds, [ability](const auto& id) {
		const auto* skill = FindSkill(id);
		return skill && skill->ability == ability;
	});
}

void PlayerSkills::ApplyPurchasedAbilities(No::Registry& registry, No::Entity player) {
	const auto* progress = registry.GetComponent<SkillProgressComponent>(player);
	if (!progress) return;
	for (const auto& id : progress->purchasedSkillIds) {
		const auto* skill = FindSkill(id);
		if (!skill) continue;
		switch (skill->ability) {
		case PlayerAbility::kAirDash: GrantTag<AirDashTag>(registry, player); break;
		case PlayerAbility::kHighJump: GrantTag<HighJumpTag>(registry, player); break;
		case PlayerAbility::kMultiJump: GrantTag<MultiJumpTag>(registry, player); break;
		case PlayerAbility::kMagicScaffold: GrantTag<CreateMagicScaffoldTag>(registry, player); break;
		case PlayerAbility::kNone: break;
		}
	}
}
