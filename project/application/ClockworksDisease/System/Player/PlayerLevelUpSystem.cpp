#include "stdafx.h"
#include "PlayerLevelUpSystem.h"
#include "../../Component/Player/PlayerMoveTags.h"
#include "../../Component/UI/UserInterfaceComponent.h"

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

			for (auto levelUpUI : registry.View<LevelUpTextComponent>()) {
				registry.AddComponent<LevelUpFrameTag>(levelUpUI);
			}

			// レベルアップ時のエフェクト
			for (auto effectEntity : registry.View<No::TransformComponent, No::EffectEmitterComponent, LevelUpEffectTag>()) {
				registry.GetComponent<No::TransformComponent>(effectEntity)->translate = registry.GetComponent<No::TransformComponent>(e)->GetWorldPosition(registry);
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

	auto* levelComponent = registry.GetComponent<LevelComponent>(e);
	for (auto& reward : levelComponent->rewards) {
		if (reward.level == level && reward.ability != PlayerAbility::kNone) {
			GrantAbility(registry, e, reward.ability);
		}
	}
}

void PlayerLevelUpSystem::GrantAbility(No::Registry& registry, No::Entity e, PlayerAbility ability) {
	switch (ability) {
	case PlayerAbility::kMultiJump:
		if (!registry.Has<MultiJumpTag>(e)) {
			registry.AddComponent<MultiJumpTag>(e);
			auto textEntity = registry.GenerateEntity();
			registry.AddComponent<No::Transform2DComponent>(textEntity)->scale = No::Vector2(468.f, 720.f);
			auto* sprite = registry.AddComponent<No::SpriteComponent>(textEntity);
			sprite->textureName = "MultiJumpHint";
			sprite->layer = 1;
			registry.AddComponent<LevelUpTextParentTag>(textEntity);
		}
		break;
	case PlayerAbility::kHighJump:
		if (!registry.Has<HighJumpTag>(e)) registry.AddComponent<HighJumpTag>(e);
		break;
	case PlayerAbility::kAirDash:
		if (!registry.Has<AirDashTag>(e)) registry.AddComponent<AirDashTag>(e);
		break;
	case PlayerAbility::kMagicScaffold:
		if (!registry.Has<CreateMagicScaffoldTag>(e)) registry.AddComponent<CreateMagicScaffoldTag>(e);
		break;
	default:
		break;
	}
}
