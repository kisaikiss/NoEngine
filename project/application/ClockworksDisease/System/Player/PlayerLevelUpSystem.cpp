#include "stdafx.h"
#include "PlayerLevelUpSystem.h"
#include "../../Component/Player/PlayerComponent.h"
#include "../../Component/Player/PlayerMoveTags.h"

REFLECT_STRUCT_BEGIN(LevelUpEffectTag)
REFLECT_STRUCT_END(LevelUpEffectTag)

void PlayerLevelUpSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	for (auto e : registry.View<No::TransformComponent, PlayerComponent, LevelComponent>()) {
		auto* levelComponent = registry.GetComponent<LevelComponent>(e);
		if (levelComponent->power >= levelComponent->nextLevelUp) {
			levelComponent->nowLevel++;
			constexpr uint32_t kAmountOfPowerNeededForTheNextLevelUp = 10;
			levelComponent->power -= levelComponent->nextLevelUp;
			levelComponent->nextLevelUp += kAmountOfPowerNeededForTheNextLevelUp;
			EnhancementsUponLevelingUp(registry, e, levelComponent->nowLevel);

			// レベルアップ時のエフェクト
			for (auto effectEntity : registry.View<No::TransformComponent, No::EffectEmitterComponent, LevelUpEffectTag>()) {
				registry.GetComponent<No::TransformComponent>(effectEntity)->translate = registry.GetComponent<No::TransformComponent>(e)->GetWorldPosition();
				registry.AddComponent<No::EffectEmitTag>(effectEntity);
			}
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
