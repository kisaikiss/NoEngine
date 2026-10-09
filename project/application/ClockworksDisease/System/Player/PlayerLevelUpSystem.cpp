#include "stdafx.h"
#include "PlayerLevelUpSystem.h"
#include "../../Component/Player/PlayerComponent.h"
#include "../../Component/Player/PlayerMoveTags.h"
#include "../../Component/UI/UserInterfaceComponent.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"
#include "../../Component/Camera/CameraIntroComponent.h"

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

void EnqueueOrShowLevelUpHint(No::Registry& registry, const std::string& textureName) {
	for (auto e : registry.View<LevelUpTextComponent>()) {
		auto* queue = registry.GetComponent<LevelUpHintQueueComponent>(e);
		if (!queue) {
			queue = registry.AddComponent<LevelUpHintQueueComponent>(e);
		}

		if (registry.Has<LevelUpFrameTag>(e)) {
			queue->pendingTextureNames.push_back(textureName);
			continue;
		}

		CreateLevelUpHintEntity(registry, textureName);
		registry.AddComponent<LevelUpFrameTag>(e);
	}
}

bool sFirstFrame = true;

}

void PlayerLevelUpSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	bool isSkip = false;
	for (auto e : registry.View<No::TransformComponent, No::CameraComponent, FollowCameraComponent>()) {
		if (registry.Has<CameraLockTag>(e)) isSkip = true; 
	}
	if (sFirstFrame && !isSkip) {
		EnqueueOrShowLevelUpHint(registry, "tutorialText");
		sFirstFrame = false;
	}

	for (auto e : registry.View<No::TransformComponent, PlayerComponent, LevelComponent>()) {
		auto* levelComponent = registry.GetComponent<LevelComponent>(e);
		// レベルが1の時に次にレベルが上がるまでの経験値を設定と一致させる
		if (levelComponent->nowLevel == 1) {
			if (!levelComponent->levelUpRequirements.empty()) {
				levelComponent->nextLevelUp = levelComponent->levelUpRequirements[0];
			}
		}

		if (levelComponent->power >= levelComponent->nextLevelUp) {
			levelComponent->nowLevel++;
			levelComponent->power -= levelComponent->nextLevelUp;
			levelComponent->nextLevelUp = GetRequirementForLevel(levelComponent, levelComponent->nowLevel);
			EnhancementsUponLevelingUp(registry, e, levelComponent->nowLevel);

			for (auto effectEntity : registry.View<No::TransformComponent, No::EffectEmitterComponent, LevelUpEffectTag>()) {
				registry.GetComponent<No::TransformComponent>(effectEntity)->translate = registry.GetComponent<No::TransformComponent>(e)->GetWorldPosition(registry);
				registry.AddComponent<No::EffectEmitTag>(effectEntity);
			}
		}
	}
}

void PlayerLevelUpSystem::EnhancementsUponLevelingUp(No::Registry& registry, No::Entity e, uint32_t level) {
	auto* player = registry.GetComponent<PlayerComponent>(e);
	player->maxStamina += player->staminaUpPerLevel;

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
#include "engine/Functions/ECS/System/SystemManager.h"
REGISTER_SYSTEM(::PlayerLevelUpSystem, "PlayerLevelUpSystem", "Gameplay")
