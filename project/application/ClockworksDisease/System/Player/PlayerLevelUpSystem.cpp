#include "stdafx.h"
#include "PlayerLevelUpSystem.h"
#include "../../Component/Player/PlayerComponent.h"
#include "../../Component/Player/PlayerMoveTags.h"
#include "../../Component/UI/UserInterfaceComponent.h"

REFLECT_STRUCT_BEGIN(LevelUpEffectTag, "ApplicationTag")
REFLECT_STRUCT_END(LevelUpEffectTag)

namespace {
// レベルアップヒントの表示要求。
// 既に別のヒントを表示中(クローズアニメーション中も含む)ならキューに積むだけにし、
// LevelUpTextSystem側でヒントが完全に右へはけたタイミングで次のヒントを取り出して表示する。
// アイドル状態(何も表示していない)ならすぐに表示を開始する。
void EnqueueOrShowLevelUpHint(No::Registry& registry, const std::string& textureName) {
	for (auto e : registry.View<LevelUpTextComponent>()) {
		auto* queue = registry.GetComponent<LevelUpHintQueueComponent>(e);
		if (!queue) {
			queue = registry.AddComponent<LevelUpHintQueueComponent>(e);
		}

		if (registry.Has<LevelUpFrameTag>(e)) {
			// 表示中(またはクローズアニメーション中)なのでキューに積むだけ
			queue->pendingTextureNames.push_back(textureName);
			continue;
		}

		// アイドル状態なのですぐに表示を開始する
		CreateLevelUpHintEntity(registry, textureName);
		registry.AddComponent<LevelUpFrameTag>(e);
	}
}
}

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
			EnqueueOrShowLevelUpHint(registry, "MultiJumpHint");
		}
		break;
	case PlayerAbility::kHighJump:
		if (!registry.Has<HighJumpTag>(e)) {
			registry.AddComponent<HighJumpTag>(e);
			if (auto* debug = registry.GetComponent<PlayerAbilityDebugComponent>(e)) {
				debug->highJump = true;
			}
			EnqueueOrShowLevelUpHint(registry, "HighJumpHint");
		}
		break;
	case PlayerAbility::kAirDash:
		if (!registry.Has<AirDashTag>(e)) {
			registry.AddComponent<AirDashTag>(e);
			if (auto* debug = registry.GetComponent<PlayerAbilityDebugComponent>(e)) {
				debug->airDash = true;
			}
			EnqueueOrShowLevelUpHint(registry, "AirDashHint");
		}
		break;
	case PlayerAbility::kMagicScaffold:
		if (!registry.Has<CreateMagicScaffoldTag>(e)) {
			registry.AddComponent<CreateMagicScaffoldTag>(e);
			if (auto* debug = registry.GetComponent<PlayerAbilityDebugComponent>(e)) {
				debug->magicScaffold = true;
			}
			EnqueueOrShowLevelUpHint(registry, "MagicHint");
		}
		break;
	default:
		break;
	}
}