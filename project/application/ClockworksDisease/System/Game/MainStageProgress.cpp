#include "MainStageProgress.h"

#include "engine/Editor/DataDriven/SceneSerializer.h"
#include "engine/Editor/EditTag.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"
#include "application/ClockworksDisease/Component/Camera/CameraIntroComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerMoveTags.h"
#include "application/ClockworksDisease/Component/UI/UserInterfaceComponent.h"


#include <unordered_set>

namespace MainStageProgress {
namespace {
nlohmann::json sMainStageSnapshot;
std::unordered_set<std::string> sCollectedItemNames;
bool sHasSnapshot = false;
bool sCaptureQueued = false;
PlayerAbilityDebugComponent sPlayerAbility{};
float sPlayerStamina = 0.0f;
}

void QueueCapture() {
	sCaptureQueued = true;
}

void Reset() {
	sMainStageSnapshot = nlohmann::json();
	sCollectedItemNames.clear();
	sHasSnapshot = false;
	sCaptureQueued = false;
}

void CaptureIfQueued(No::Registry& registry) {
	if (!sCaptureQueued) return;
	sMainStageSnapshot = NoEngine::Editor::SaveScene(registry);

	// 現状のプレイヤーの状態を保持
	for (auto e : registry.View<PlayerAbilityDebugComponent, PlayerComponent>()) {
		sPlayerAbility = *registry.GetComponent<PlayerAbilityDebugComponent>(e);
		sPlayerStamina = registry.GetComponent<PlayerComponent>(e)->maxStamina;
	}
	sHasSnapshot = true;
	sCaptureQueued = false;

}

bool Restore(No::Registry& registry) {
	if (!sHasSnapshot) return false;
	NoEngine::Editor::LoadScene(registry, sMainStageSnapshot, true);

	// 読み込み元のシーンファイルに存在する取得済みアイテムを再生成させない。
	std::vector<No::Entity> collectedEntities;
	for (auto entity : registry.View<No::EditTag>()) {
		const auto* editTag = registry.GetComponent<No::EditTag>(entity);
		if (editTag && sCollectedItemNames.contains(editTag->name)) {
			collectedEntities.push_back(entity);
		}
	}

	// メインカメラのロックタグを外してプレイヤーが移動できるようにしておく
	for (auto e : registry.View<No::TransformComponent, No::CameraComponent, FollowCameraComponent>()) {
		if (registry.Has<CameraLockTag>(e)) {
			registry.RemoveComponent<CameraLockTag>(e);
		}
	}

	// プレイヤーを見えるようにする
	for (auto e : registry.View<No::MeshComponent, No::MaterialComponent, PlayerComponent>()) {
		registry.GetComponent<No::MeshComponent>(e)->isVisible = true;
		registry.GetComponent<No::MaterialComponent>(e)->castShadow = true;
	}

	// レベルアップ時のUIが表示しっぱなしのときに非表示に戻す
	for (auto e : registry.View<No::SpriteComponent, LevelUpTextComponent>()) {
		registry.GetComponent<No::SpriteComponent>(e)->isVisible = false;
	}

	for (auto entity : collectedEntities) registry.DestroyEntity(entity);
	return true;
}

void RecordCollectedItem(No::Registry& registry, No::Entity item) {
	if (No::GetCurrentSceneName(registry) != "GameScene") return;
	const auto* editTag = registry.GetComponent<No::EditTag>(item);
	if (editTag) sCollectedItemNames.insert(editTag->name);
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

	if (No::GetCurrentSceneName(registry) == "GameScene") {
		restored_ = MainStageProgress::Restore(registry);
	} else {
		// メインステージでのプレイヤーのアビリティを入れる
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
			registry.GetComponent<PlayerComponent>(e)->maxStamina = MainStageProgress::sPlayerStamina;
		}
	}
}

void MainStageProgressCaptureSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	if (No::GetCurrentSceneName(registry) == "GameScene") {
		MainStageProgress::CaptureIfQueued(registry);
	}
}
