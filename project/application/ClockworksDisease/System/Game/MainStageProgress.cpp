#include "MainStageProgress.h"

#include "engine/Editor/DataDriven/SceneSerializer.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"
#include "application/ClockworksDisease/Component/Camera/CameraIntroComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "application/ClockworksDisease/Component/Player/SkillProgressComponent.h"
#include "application/ClockworksDisease/Component/Game/StageTransitionComponent.h"
#include "application/ClockworksDisease/Component/UI/UserInterfaceComponent.h"

namespace MainStageProgress {
namespace {
nlohmann::json sMainStageSnapshot;
std::unordered_map<std::string, nlohmann::json> sStageSnapshots;
std::unordered_map<std::string, std::unordered_set<std::string>> sCollectedItemNames;
bool sCaptureQueued = false;
std::unordered_set<std::string> sReachedStages;
float sPlayerStamina = 0.0f;
float sPlayerCurrentStamina = 0.0f;
LevelComponent sPlayerLevel{};
SkillProgressComponent sPlayerSkills{};
bool sHasSharedProgress = false;
REGISTER_PLAY_MODE_STATIC(sMainStageSnapshot);
REGISTER_PLAY_MODE_STATIC(sStageSnapshots);
REGISTER_PLAY_MODE_STATIC(sCollectedItemNames);
REGISTER_PLAY_MODE_STATIC(sCaptureQueued);
REGISTER_PLAY_MODE_STATIC(sReachedStages);
REGISTER_PLAY_MODE_STATIC(sPlayerStamina);
REGISTER_PLAY_MODE_STATIC(sPlayerCurrentStamina);
REGISTER_PLAY_MODE_STATIC(sPlayerLevel);
REGISTER_PLAY_MODE_STATIC(sPlayerSkills);
REGISTER_PLAY_MODE_STATIC(sHasSharedProgress);
}

bool HasReachedStage(const std::string& sceneName) {
	return sReachedStages.contains(sceneName);
}

void QueueCapture() {
	sCaptureQueued = true;
}

void Reset() {
	sMainStageSnapshot = nlohmann::json();
	sStageSnapshots.clear();
	sCollectedItemNames.clear();
	sCaptureQueued = false;
	sReachedStages.clear();
	sPlayerLevel = {};
	sPlayerSkills = {};
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
	for (auto e : registry.View<PlayerComponent, LevelComponent>()) {
		const auto* skills = registry.GetComponent<SkillProgressComponent>(e);
		sPlayerSkills = skills ? *skills : SkillProgressComponent{};
		const auto* player = registry.GetComponent<PlayerComponent>(e);
		sPlayerStamina = player->maxStamina;
		sPlayerCurrentStamina = player->stamina;
		sPlayerLevel = *registry.GetComponent<LevelComponent>(e);
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

void MainStageProgressResetSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(registry);
	static_cast<void>(deltaTime);
	if (reset_) return;
	MainStageProgress::Reset();
	reset_ = true;
}

REGISTER_SYSTEM(::MainStageProgressResetSystem, "MainStageProgressResetSystem", "Gameplay")

void MainStageProgressRestoreSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	if (isFirstFrame_) {
		isFirstFrame_ = false;
	} else {
		return;
	}
	
	if (restored_) return;

	const auto sceneName = No::GetCurrentSceneName(registry);
	if (!sceneName.empty()) MainStageProgress::sReachedStages.insert(sceneName);
	restored_ = MainStageProgress::RestoreCurrentStage(registry);
	if (MainStageProgress::sHasSharedProgress) {
		// ステージ固有の保存状態を復元した後、全ステージ共通の育成状況を適用する。
		for (auto e : registry.View<PlayerComponent, LevelComponent>()) {
			auto* skills = registry.GetComponent<SkillProgressComponent>(e);
			if (!skills) skills = registry.AddComponent<SkillProgressComponent>(e);
			*skills = MainStageProgress::sPlayerSkills;
			PlayerSkills::ApplyPurchasedAbilities(registry, e);
			auto* player = registry.GetComponent<PlayerComponent>(e);
			player->maxStamina = MainStageProgress::sPlayerStamina;
			player->stamina = std::min(MainStageProgress::sPlayerCurrentStamina, player->maxStamina);
			auto* level = registry.GetComponent<LevelComponent>(e);
			level->power = MainStageProgress::sPlayerLevel.power;
			level->nowLevel = MainStageProgress::sPlayerLevel.nowLevel;
			level->nextLevelUp = MainStageProgress::sPlayerLevel.nextLevelUp;
		}
	}
}

void MainStageProgressCaptureSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	MainStageProgress::CaptureIfQueued(registry);
}

REGISTER_SYSTEM(::MainStageProgressRestoreSystem, "MainStageProgressRestoreSystem", "Gameplay")
REGISTER_SYSTEM(::MainStageProgressCaptureSystem, "MainStageProgressCaptureSystem", "Gameplay")
