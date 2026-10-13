#include "stdafx.h"
#include "PlayModeController.h"

#include "../Event/SceneChangeEvent.h"
#include "../../Scene/SceneNameComponent.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"
#include "engine/Editor/EditTag.h"

namespace NoEngine {
namespace ECS {

namespace {
#ifdef USE_IMGUI
bool sGameStop = true;
#else
bool sGameStop = false;
#endif

PlayModeController::State sState = PlayModeController::State::kEditing;

nlohmann::json sSnapshot;
bool sHasSnapshot = false;
std::string sStartSceneName;
bool sRestoreAfterSceneChange = false; // 再生中にシーンが変わり、Stop後に元シーンへ戻している最中

std::string GetSceneName(Registry& registry) {
	std::string name;
	for (auto e : registry.View<SceneNameComponent>()) {
		name = registry.GetComponent<SceneNameComponent>(e)->GetName();
	}
	return name;
}

// スナップショットに存在しない(再生中に生成された)Entityを削除し、
// スナップショットの内容を既存Entityへ書き戻す。
void RestoreSnapshot(Registry& registry) {
	std::unordered_set<std::string> snapshotNames;
	if (sSnapshot.contains("entities")) {
		for (auto& [name, j] : sSnapshot["entities"].items()) {
			snapshotNames.insert(name);
		}
	}

	std::vector<Entity> toDestroy;
	for (auto e : registry.View<Editor::EditTag>()) {
		auto* tag = registry.GetComponent<Editor::EditTag>(e);
		if (tag && !snapshotNames.contains(tag->name)) {
			toDestroy.push_back(e);
		}
	}
	for (auto e : toDestroy) {
		registry.DestroyEntity(e);
	}
	registry.FlushDestroy();

	Editor::LoadScene(registry, sSnapshot);
}
}

PlayModeController::State PlayModeController::GetState() { return sState; }
bool PlayModeController::IsInPlayMode() { return sState != State::kEditing; }
bool PlayModeController::IsGameStopped() { return sGameStop; }
const nlohmann::json& PlayModeController::GetSnapshot() { return sSnapshot; }

void PlayModeController::Play(Registry& registry) {
	if (sState == State::kEditing) {
		// 停止状態から再生する瞬間だけスナップショットを取得する(一時停止からの再開では撮り直さない)
		sSnapshot = Editor::SaveScene(registry);
		sHasSnapshot = true;
		sStartSceneName = GetSceneName(registry);
	}
	sState = State::kPlaying;
	sGameStop = false;
}

void PlayModeController::Pause() {
	sGameStop = true;
	if (sState == State::kPlaying) {
		sState = State::kPaused;
	}
}

void PlayModeController::Stop(Registry& registry) {
	sGameStop = true;

	if (sState != State::kEditing && sHasSnapshot) {
		if (GetSceneName(registry) != sStartSceneName) {
			// 再生中にシーンが切り替わっていたら、元のシーンへ戻す。
			// スナップショットの復元は切替後のシーンでUpdate()が行う。
			Event::SceneChangeEvent event;
			event.nextScene = sStartSceneName;
			event.transitionType = Event::SceneTransitionType::kImmediate;
			registry.EmitEvent(event);
			sRestoreAfterSceneChange = true;
		} else {
			RestoreSnapshot(registry);
		}
	}

	sState = State::kEditing;
}

void PlayModeController::Update(Registry& registry) {
	if (sRestoreAfterSceneChange) {
		RestoreSnapshot(registry);
		sRestoreAfterSceneChange = false;
	}
}

}
}