#include "stdafx.h"
#include "PlayerLevelUpSystem.h"
#include "../../Component/Player/PlayerComponent.h"
#include "../../Component/Player/PlayerMoveTags.h"

void PlayerLevelUpSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	for (auto e : registry.View<PlayerComponent, LevelComponent>()) {
		auto* levelComponent = registry.GetComponent<LevelComponent>(e);
		if (levelComponent->power >= levelComponent->nextLevelUp) {
			levelComponent->nowLevel++;
			constexpr uint32_t kAmountOfPowerNeededForTheNextLevelUp = 10;
			levelComponent->power -= levelComponent->nextLevelUp;
			levelComponent->nextLevelUp += kAmountOfPowerNeededForTheNextLevelUp;
			EnhancementsUponLevelingUp(registry, e, levelComponent->nowLevel);
		}
	}
}

void PlayerLevelUpSystem::EnhancementsUponLevelingUp(No::Registry& registry, No::Entity e, uint32_t level) {
	// 全レベル共通
	// スタミナ最大値を上昇させる
	constexpr float kAmountOfMaxStaminaUp = 1.0f;
	registry.GetComponent<PlayerComponent>(e)->maxStamina += kAmountOfMaxStaminaUp;

	// level2
	if (level == 2) {
		// 空中ジャンプを追加
		if (!registry.Has<MultiJumpTag>(e)) {
			registry.AddComponent<MultiJumpTag>(e);
		}
	}
}
