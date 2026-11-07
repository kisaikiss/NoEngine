#include "stdafx.h"
#include "PlayerLevelUpSystem.h"
#include "../../Component/Player/PlayerComponent.h"
#include "../../Component/Player/SkillProgressComponent.h"

REFLECT_STRUCT_BEGIN(LevelUpEffectTag, "ApplicationTag")
REFLECT_STRUCT_END(LevelUpEffectTag)

namespace {
// レベルアップ後に次のレベルまでに必要な経験値をテーブルから引く。
// テーブルが空、またはインデックスが範囲外なら最後の値、もしくはフォールバック値を使う。
uint32_t GetRequirementForLevel(const LevelComponent* levelComponent, uint32_t level) {
	if (levelComponent->levelUpRequirements.empty()) {
		return levelComponent->nextLevelUp; // テーブル未設定なら現状維持
	}
	size_t index = (level >= 1) ? static_cast<size_t>(level - 1) : 0;
	if (index >= levelComponent->levelUpRequirements.size()) {
		index = levelComponent->levelUpRequirements.size() - 1;
	}
	return levelComponent->levelUpRequirements[index];
}

}

void PlayerLevelUpSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	for (auto e : registry.View<No::TransformComponent, PlayerComponent, LevelComponent>()) {
		auto* levelComponent = registry.GetComponent<LevelComponent>(e);
		auto* skillProgress = registry.GetComponent<SkillProgressComponent>(e);
		if (!skillProgress) skillProgress = registry.AddComponent<SkillProgressComponent>(e);
		// レベルが1の時に次にレベルが上がるまでの経験値を設定と一致させる
		if (levelComponent->nowLevel == 1) {
			if (!levelComponent->levelUpRequirements.empty()) {
				levelComponent->nextLevelUp = levelComponent->levelUpRequirements[0];
			}
		}

		if (levelComponent->nextLevelUp > 0 && levelComponent->power >= levelComponent->nextLevelUp) {
			levelComponent->nowLevel++;
			levelComponent->power -= levelComponent->nextLevelUp;
			levelComponent->nextLevelUp = GetRequirementForLevel(levelComponent, levelComponent->nowLevel);
			++skillProgress->skillPoints;

			for (auto effectEntity : registry.View<No::TransformComponent, No::EffectEmitterComponent, LevelUpEffectTag>()) {
				registry.GetComponent<No::TransformComponent>(effectEntity)->translate = registry.GetComponent<No::TransformComponent>(e)->GetWorldPosition(registry);
				registry.AddComponent<No::EffectEmitTag>(effectEntity);
			}
		}
	}
}

REGISTER_SYSTEM(::PlayerLevelUpSystem, "PlayerLevelUpSystem", "Gameplay")
