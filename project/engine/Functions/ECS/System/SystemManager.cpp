#include "SystemManager.h"
#include "../Component/Common/PauseComponent.h"
#include "../Event/SceneChangeEvent.h"
#include "../../Scene/SceneNameComponent.h"
#include "../../Particle/ParticleManager.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"
#include "engine/Editor/EditTag.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

#include <unordered_set>

namespace NoEngine {
namespace ECS {

namespace {
#ifdef USE_IMGUI
bool sGameStop = true;
#else
bool sGameStop = false;
#endif // USE_IMGUI

// エディタの再生状態
enum class EditorPlayState {
	kEditing, // 未再生、または"■"で停止済み
	kPlaying, // ">"で再生中
	kPaused,  // 再生中に"||"で一時停止した状態
};
EditorPlayState sPlayState = EditorPlayState::kEditing;

// ">"を押した瞬間(kEditing -> kPlaying)にだけ取得するシーンのスナップショット。
// "■"を押したときにこの状態へ復元する。
nlohmann::json sPlaySnapshot;
bool sHasPlaySnapshot = false;
}

SystemManager::SystemManager() {}

bool SystemManager::IsInPlayMode() {
	return sPlayState != EditorPlayState::kEditing;
}

const nlohmann::json& SystemManager::GetPlaySnapshot() {
	return sPlaySnapshot;
}

void SystemManager::UpdateAll(ComputeContext& ctx, Registry& registry, float deltaTime) {
	auto pauseView = registry.View<PauseComponent>();
	bool isPause = false;
	for (auto entity : pauseView) {
		auto* pauseComp = registry.GetComponent<PauseComponent>(entity);
		isPause = pauseComp->isPause;
	}

	for (auto& system : systems_) {
		if (!sGameStop || !system->GetStopInGameStop()) {
			if (!isPause || !system->GetStopInPause()) {
				system->Update(ctx, registry, deltaTime);
			}
		}
	}
#ifdef USE_IMGUI

	ImGui::Begin("GameController", nullptr, ImGuiWindowFlags_NoTitleBar);
	float windowWidth = ImGui::GetWindowSize().x;
	float itemWidth = ImGui::CalcTextSize("Button").x + ImGui::GetStyle().FramePadding.x * 2;

	ImGui::SetCursorPosX(windowWidth * 0.5f - itemWidth);
	if (ImGui::Button("■")) {
		sGameStop = true;

		if (sPlayState != EditorPlayState::kEditing && sHasPlaySnapshot) {
			// ">"を押す直前の状態へ復元する。
			// スナップショットに存在しない(再生中に生成された)Entityは先に削除してから、
			// スナップショットの内容を既存Entityへ書き戻す。
			std::unordered_set<std::string> snapshotNames;
			if (sPlaySnapshot.contains("entities")) {
				for (auto& [name, j] : sPlaySnapshot["entities"].items()) {
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

			Editor::LoadScene(registry, sPlaySnapshot);
		} else {
			// スナップショットが無い場合は従来通りシーンをリロードする(フォールバック)
			auto sceneView = registry.View<SceneNameComponent>();
			for (auto e : sceneView) {
				auto* sceneName = registry.GetComponent<SceneNameComponent>(e);
				Event::SceneChangeEvent event;
				event.nextScene = sceneName->GetName();
				registry.EmitEvent(event);
			}
		}

		sPlayState = EditorPlayState::kEditing;
	}
	ImGui::SameLine();
	if (ImGui::Button(">")) {
		if (sPlayState == EditorPlayState::kEditing) {
			// 停止状態から再生する瞬間だけスナップショットを取得する
			// (一時停止からの再開では撮り直さない)
			sPlaySnapshot = Editor::SaveScene(registry);
			sHasPlaySnapshot = true;
		}
		sPlayState = EditorPlayState::kPlaying;
		sGameStop = false;
	}
	ImGui::SameLine();
	if (ImGui::Button("||")) {
		sGameStop = true;
		if (sPlayState == EditorPlayState::kPlaying) {
			sPlayState = EditorPlayState::kPaused;
		}
	}

	ImGui::End();
#endif // USE_IMGUI

}
}
}