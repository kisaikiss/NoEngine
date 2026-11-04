#include "stdafx.h"
#include "PlayModeController.h"
#include "PlayModeStaticState.h"
#include "SystemManager.h"

#include "../../Scene/SceneNameComponent.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"
#include "engine/Editor/Commands/EditorCommandOperator.h"

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
std::unique_ptr<Registry> sRegistrySnapshot;
std::unique_ptr<SystemManager::Snapshot> sSystemSnapshot;
std::string sStartSceneName;
bool sRestoreAfterSceneChange = false; // 再生中にシーンが変わり、Stop後に元シーンへ戻している最中
std::optional<std::string> sSceneRestoreRequest;

std::string GetSceneName(Registry& registry) {
	std::string name;
	for (auto e : registry.View<SceneNameComponent>()) {
		name = registry.GetComponent<SceneNameComponent>(e)->GetName();
	}
	return name;
}

// コンポーネントへの古いポインタを持つ編集履歴を破棄してから、全状態を入れ替える。
void RestoreSnapshot(Registry& registry, SystemManager* systems) {
	if (sSystemSnapshot && !systems) throw std::logic_error("SystemManager required to restore play mode");
	Editor::EditorCommandOperator::Reset();
	registry = std::move(*sRegistrySnapshot);
	sRegistrySnapshot.reset();
	PlayModeStaticStates::Get().Restore();
	if (sSystemSnapshot) systems->RestoreSnapshot(std::move(sSystemSnapshot), registry);
}
}

PlayModeController::State PlayModeController::GetState() { return sState; }
bool PlayModeController::IsInPlayMode() { return sState != State::kEditing; }
bool PlayModeController::IsGameStopped() { return sGameStop; }
const nlohmann::json& PlayModeController::GetSnapshot() { return sSnapshot; }

void PlayModeController::Play(Registry& registry, SystemManager* systems) {
	if (sRestoreAfterSceneChange || sSceneRestoreRequest) return;
	if (sState == State::kEditing) {
		// 停止状態から再生する瞬間だけスナップショットを取得する(一時停止からの再開では撮り直さない)
		auto snapshot = registry.Clone();
		auto systemSnapshot = systems ? systems->CaptureSnapshot() : nullptr;
		sSnapshot = Editor::SaveScene(registry);
		sStartSceneName = GetSceneName(registry);
		PlayModeStaticStates::Get().Capture();
		sRegistrySnapshot = std::move(snapshot);
		sSystemSnapshot = std::move(systemSnapshot);
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

void PlayModeController::Stop(Registry& registry, SystemManager* systems) {
	if (sSystemSnapshot && !systems) throw std::logic_error("SystemManager required to stop play mode");
	sGameStop = true;

	if (sState != State::kEditing && sRegistrySnapshot) {
		// 通常のシーン変更イベントとは別に要求し、遷移演出中でも元シーンへ戻せるようにする。
		sSceneRestoreRequest = sStartSceneName;
		if (GetSceneName(registry) != sStartSceneName) {
			sRestoreAfterSceneChange = true;
		} else {
			RestoreSnapshot(registry, systems);
		}
	}

	sState = State::kEditing;
}

std::optional<std::string> PlayModeController::TakeSceneRestoreRequest() {
	auto request = std::move(sSceneRestoreRequest);
	sSceneRestoreRequest.reset();
	return request;
}

void PlayModeController::Update(Registry& registry, SystemManager* systems) {
	if (sRestoreAfterSceneChange && GetSceneName(registry) == sStartSceneName) {
		RestoreSnapshot(registry, systems);
		sRestoreAfterSceneChange = false;
	}
}

}
}
