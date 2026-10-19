#include "MainStageProgress.h"

#include "engine/Editor/DataDriven/SceneSerializer.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"
#include "application/ClockworksDisease/Component/Camera/CameraIntroComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerMoveTags.h"
#include "application/ClockworksDisease/Component/Game/StageTransitionComponent.h"
#include "application/ClockworksDisease/Component/UI/UserInterfaceComponent.h"

namespace MainStageProgress {
namespace {
nlohmann::json sMainStageSnapshot;
std::unordered_map<std::string, nlohmann::json> sStageSnapshots;
std::unordered_map<std::string, std::unordered_set<std::string>> sCollectedItemNames;
bool sCaptureQueued = false;
PlayerAbilityDebugComponent sPlayerAbility{};
float sPlayerStamina = 0.0f;
float sPlayerCurrentStamina = 0.0f;
LevelComponent sPlayerLevel{};
bool sHasSharedProgress = false;
}

void QueueCapture() {
	sCaptureQueued = true;
}

void Reset() {
	sMainStageSnapshot = nlohmann::json();
	sStageSnapshots.clear();
	sCollectedItemNames.clear();
	sCaptureQueued = false;
	sPlayerAbility = {};
	sPlayerLevel = {};
	sPlayerStamina = 0.0f;
	sPlayerCurrentStamina = 0.0f;
	sHasSharedProgress = false;
}

void CaptureIfQueued(No::Registry& registry) {
	if (!sCaptureQueued) return;
	const std::string sceneName = No::GetCurrentSceneName(registry);
	if (sceneName.empty()) return;
	sMainStageSnapshot = NoEngine::Editor::SaveScene(registry);
	sStageSnapshots[sceneName] = sMainStageSnapshot;

	// 現状のプレイヤーの状態を保持
	for (auto e : registry.View<PlayerAbilityDebugComponent, PlayerComponent>()) {
		sPlayerAbility = *registry.GetComponent<PlayerAbilityDebugComponent>(e);
		const auto* player = registry.GetComponent<PlayerComponent>(e);
		sPlayerStamina = player->maxStamina;
		sPlayerCurrentStamina = player->stamina;
		if (const auto* level = registry.GetComponent<LevelComponent>(e)) sPlayerLevel = *level;
		sHasSharedProgress = true;
	}
	sCaptureQueued = false;

}

bool Restore(No::Registry& registry) {
	return RestoreCurrentStage(registry);
}

bool RestoreCurrentStage(No::Registry& registry) {
	const std::string sceneName = No::GetCurrentSceneName(registry);
	const auto snapshot = sStageSnapshots.find(sceneName);
	if (snapshot == sStageSnapshots.end()) return false;
	NoEngine::Editor::LoadScene(registry, snapshot->second, true);

	// 読み込み元のシーンファイルに存在する取得済みアイテムを再生成させない。
	std::vector<No::Entity> collectedEntities;
	for (auto entity : registry.View<No::EditTag>()) {
		const auto* editTag = registry.GetComponent<No::EditTag>(entity);
		const auto collected = sCollectedItemNames.find(sceneName);
		if (editTag && collected != sCollectedItemNames.end() && collected->second.contains(editTag->name)) {
			collectedEntities.push_back(entity);
		}
	}

	// メインカメラのロックタグを外してプレイヤーが移動できるようにしておく
	for (auto e : registry.View<No::TransformComponent, No::CameraComponent, FollowCameraComponent>()) {
		if (registry.Has<CameraLockTag>(e)) {
			registry.RemoveComponent<CameraLockTag>(e);
		}
	}


	// レベルアップ時のUIが表示しっぱなしのときに非表示に戻す
	for (auto e : registry.View<No::SpriteComponent, LevelUpTextComponent>()) {
		registry.GetComponent<No::SpriteComponent>(e)->isVisible = false;
	}

	// ワープブロックを復活させる
	No::Vector3 playerNewPosition = No::Vector3::ZERO;
	for (auto e : registry.View<StageTransitionComponent, No::TransformComponent>()) {
		registry.GetComponent<No::TransformComponent>(e)->scale = No::Vector3::UNIT_SCALE;
		playerNewPosition = registry.GetComponent<StageTransitionComponent>(e)->returnPosition;
		if (!registry.Has<No::SphereCollider>(e)) {
			registry.AddComponent<No::SphereCollider>(e);
		}
	}

	// プレイヤーを見えるようにし、ワープブロックが指定した位置へ移動
	for (auto e : registry.View<No::MeshComponent, No::MaterialComponent, PlayerComponent>()) {
		registry.GetComponent<No::MeshComponent>(e)->isVisible = true;
		registry.GetComponent<No::MaterialComponent>(e)->castShadow = true;
		registry.GetComponent<No::TransformComponent>(e)->SetWorldPosition(registry, playerNewPosition);
	}

	for (auto entity : collectedEntities) registry.DestroyEntity(entity);
	return true;
}

void RecordCollectedItem(No::Registry& registry, No::Entity item) {
	const std::string sceneName = No::GetCurrentSceneName(registry);
	const auto* editTag = registry.GetComponent<No::EditTag>(item);
	if (editTag && !sceneName.empty()) sCollectedItemNames[sceneName].insert(editTag->name);
}
}

void MainStageProgressRestoreSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	if (isFirstFrame_) {
		isFirstFrame_ = false;
	} else {
		return;
	}
	
	if (restored_) return;

	restored_ = MainStageProgress::RestoreCurrentStage(registry);
	if (MainStageProgress::sHasSharedProgress) {
		// ステージ固有の保存状態を復元した後、全ステージ共通の育成状況を適用する。
		for (auto e : registry.View<PlayerComponent, PlayerAbilityDebugComponent>()) {
			auto* playerAbility = registry.GetComponent<PlayerAbilityDebugComponent>(e);
			playerAbility->airDash = MainStageProgress::sPlayerAbility.airDash;
			playerAbility->highJump = MainStageProgress::sPlayerAbility.highJump;
			playerAbility->magicScaffold = MainStageProgress::sPlayerAbility.magicScaffold;
			if (playerAbility->airDash && !registry.Has<AirDashTag>(e)) {
				registry.AddComponent<AirDashTag>(e);
			}

			if (playerAbility->highJump && !registry.Has<HighJumpTag>(e)) {
				registry.AddComponent<HighJumpTag>(e);
			}

			if (playerAbility->magicScaffold && !registry.Has<CreateMagicScaffoldTag>(e)) {
				registry.AddComponent<CreateMagicScaffoldTag>(e);
			}
			auto* player = registry.GetComponent<PlayerComponent>(e);
			player->maxStamina = MainStageProgress::sPlayerStamina;
			player->stamina = std::min(MainStageProgress::sPlayerCurrentStamina, player->maxStamina);
			if (auto* level = registry.GetComponent<LevelComponent>(e)) {
				level->power = MainStageProgress::sPlayerLevel.power;
				level->nowLevel = MainStageProgress::sPlayerLevel.nowLevel;
				level->nextLevelUp = MainStageProgress::sPlayerLevel.nextLevelUp;
				// 各ステージで定義された報酬表を使い、共通レベルまでに解放した能力を再付与する。
				for (const auto& reward : level->rewards) {
					if (reward.level > level->nowLevel) continue;
					switch (reward.ability) {
					case PlayerAbility::kMultiJump:
						if (!registry.Has<MultiJumpTag>(e)) registry.AddComponent<MultiJumpTag>(e);
						break;
					case PlayerAbility::kHighJump:
						playerAbility->highJump = true;
						if (!registry.Has<HighJumpTag>(e)) registry.AddComponent<HighJumpTag>(e);
						break;
					case PlayerAbility::kAirDash:
						playerAbility->airDash = true;
						if (!registry.Has<AirDashTag>(e)) registry.AddComponent<AirDashTag>(e);
						break;
					case PlayerAbility::kMagicScaffold:
						playerAbility->magicScaffold = true;
						if (!registry.Has<CreateMagicScaffoldTag>(e)) registry.AddComponent<CreateMagicScaffoldTag>(e);
						break;
					default:
						break;
					}
				}
			}
		}
	}
}

void MainStageProgressCaptureSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	MainStageProgress::CaptureIfQueued(registry);
}

REGISTER_SYSTEM(::MainStageProgressRestoreSystem, "MainStageProgressRestoreSystem", "Gameplay")
REGISTER_SYSTEM(::MainStageProgressCaptureSystem, "MainStageProgressCaptureSystem", "Gameplay")
