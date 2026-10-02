#include "MainStageProgress.h"

#include "engine/Editor/DataDriven/SceneSerializer.h"
#include "engine/Editor/EditTag.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"
#include "application/ClockworksDisease/Component/Camera/CameraIntroComponent.h"

#include <unordered_set>

namespace MainStageProgress {
namespace {
nlohmann::json sMainStageSnapshot;
std::unordered_set<std::string> sCollectedItemNames;
bool sHasSnapshot = false;
bool sCaptureQueued = false;
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
		if (registry.Has<CameraIntroLockTag>(e)) {
			registry.RemoveComponent<CameraIntroLockTag>(e);
		}
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
	if (restored_ || No::GetCurrentSceneName(registry) != "GameScene") return;
	restored_ = MainStageProgress::Restore(registry);
}

void MainStageProgressCaptureSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	if (No::GetCurrentSceneName(registry) == "GameScene") {
		MainStageProgress::CaptureIfQueued(registry);
	}
}
