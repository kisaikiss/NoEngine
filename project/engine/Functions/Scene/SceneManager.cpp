#include "SceneManager.h"
#include "engine/Editor/Panels/EditorWindowManager.h"

#include "CircleScaleTransitionEffect.h"
#include "FadeTransitionEffect.h"
#include "engine/Editor/Commands/EditorCommandOperator.h"
#include "engine/Editor/DataDriven/PrefabSerializer.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "engine/Functions/ECS/Component/Common/CameraComponent.h"
#include "engine/Functions/ECS/System/SystemSceneConfig.h"
#include "engine/Functions/ECS/Event/SceneReloadEvent.h"
#include "engine/Functions/ECS/System/PlayModeController.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"
#include <fstream>
#include <filesystem>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

namespace NoEngine {
namespace Scene {
namespace {
const char* kSceneListPath = "resources/game/application.json";

bool IsValidSceneName(const std::string& name) {
	if (name.empty() || name == "." || name == "..") return false;
	return name.find_first_of("/\\:*?\"<>|") == std::string::npos && name.back() != '.' && name.back() != ' ';
}
}

SceneManager::SceneManager() {
	RegisterTransitionEffect("CircleScale", [] { return std::make_unique<CircleScaleTransitionEffect>(); });
	RegisterTransitionEffect("Fade", [] { return std::make_unique<FadeTransitionEffect>(); });
	std::ifstream list(kSceneListPath);
	if (list.is_open()) {
		try {
			nlohmann::json names; list >> names;
			for (const auto& name : names.value("scenes", std::vector<std::string>{})) {
				if (IsValidSceneName(name) && std::filesystem::is_regular_file(ECS::SystemSceneConfig::GetScenePath(name))) sceneNames_.insert(name);
			}
		} catch (...) { LogWarning("Failed to read application scene list."); }
	}
}

std::unique_ptr<ITransitionEffect> SceneManager::CreateTransitionEffect(
	Event::SceneTransitionType type, const std::string& customName) {
	std::string key;
	switch (type) {
	case Event::SceneTransitionType::kCircleScale: key = "CircleScale"; break;
	case Event::SceneTransitionType::kFade:        key = "Fade"; break;
	case Event::SceneTransitionType::kCustom:      key = customName; break;
	default: break;
	}
	auto it = transitionFactories_.find(key);
	if (it == transitionFactories_.end()) {
		LogWarning("SceneManager: transition '" + key + "' not found. fallback to CircleScale.");
		it = transitionFactories_.find("CircleScale");
	}
	return it->second();
}

SceneManager::~SceneManager() {
	Editor::EditorCommandOperator::Reset();
	Editor::SetEditPrefabCallback({});
}

std::unique_ptr<Scene> SceneManager::LoadScene(const std::string& name) {
	if (!IsValidSceneName(name)) return nullptr;
	auto next = std::make_unique<Scene>(name);
	if (!next->Load()) return nullptr;
	return next;
}

void SceneManager::CommitScene(std::unique_ptr<Scene> next) {
	// 読み込み成功後、古いRegistryやSystemを参照する履歴とコールバックを破棄する。
	Editor::EditorCommandOperator::Reset();
	Editor::SetEditPrefabCallback({});
	currentScene_ = std::move(next);
}

void SceneManager::ResetTransition(bool finishEffect) {
	if (finishEffect && currentEffect_ && currentScene_) currentEffect_->OnFinished(currentScene_->GetRegistry());
	currentEffect_.reset();
	pendingScene_.reset();
	transitionPhase_ = TransitionPhase::None;
	transitionTimer_ = 0.0f;
}

bool SceneManager::ChangeScene(const std::string& name, bool immediate,
	Event::SceneTransitionType transitionType, const std::string& customTransitionName) {
	if (!sceneNames_.contains(name) || transitionPhase_ != TransitionPhase::None) return false;
	// 演出を始める前に検証し、失敗時は現在のシーンと履歴を維持する。
	auto next = LoadScene(name);
	if (!next) return false;
	if (immediate || !currentScene_) {
		CommitScene(std::move(next));
		return true;
	}
	pendingScene_ = std::move(next);
	currentEffect_ = CreateTransitionEffect(transitionType, customTransitionName);
	transitionPhase_ = TransitionPhase::FadingOut;
	transitionTimer_ = 0.0f;
	if (currentEffect_) currentEffect_->OnFadingOutStart(currentScene_->GetRegistry());
	return true;
}

bool SceneManager::ReloadScene() {
	if (!currentScene_) return false;
	auto next = LoadScene(currentScene_->GetName());
	if (!next) return false;
	ResetTransition();
	CommitScene(std::move(next));
	return true;
}

bool SceneManager::SaveSceneList(const std::set<std::string>& names) {
	nlohmann::json data;
	data["scenes"] = names;
	std::ofstream out(kSceneListPath);
	if (out) out << data.dump(4);
	out.close();
	if (!out) LogWarning("Failed to save application scene list.");
	return static_cast<bool>(out);
}

bool SceneManager::SaveNewScene(const std::string& name, const nlohmann::json& data) {
	if (!IsValidSceneName(name) || sceneNames_.contains(name) || transitionPhase_ != TransitionPhase::None) return false;
	const auto path = ECS::SystemSceneConfig::GetScenePath(name);
	try {
		if (std::filesystem::exists(path)) return false;
		std::filesystem::create_directories(std::filesystem::path(path).parent_path());
		std::ofstream out(path);
		if (out) out << data.dump(4);
		out.close();
		if (!out) return false;
		auto next = LoadScene(name);
		auto names = sceneNames_;
		names.insert(name);
		if (!next || !SaveSceneList(names)) {
			std::filesystem::remove(path);
			return false;
		}
		sceneNames_ = std::move(names);
		CommitScene(std::move(next));
		return true;
	} catch (const std::exception& error) {
		LogWarning(std::string("Failed to create scene: ") + error.what());
		return false;
	}
}

bool SceneManager::CreateScene(const std::string& name) {
	// カメラは新規作成時にだけ保存し、既存シーンを開く際は追加しない。
	ECS::Registry registry;
	const auto mainCamera = registry.GenerateEntity();
	registry.AddComponent<Component::ActiveCameraTag>(mainCamera);
	registry.AddComponent<Component::CameraComponent>(mainCamera);
	registry.AddComponent<Component::TransformComponent>(mainCamera);
	registry.AddComponent<Editor::EditTag>(mainCamera)->name = "MainCamera";
	const auto debugCamera = registry.GenerateEntity();
	registry.AddComponent<Component::DebugCameraComponent>(debugCamera);
	registry.AddComponent<Component::CameraComponent>(debugCamera);
	registry.AddComponent<Component::TransformComponent>(debugCamera);
	registry.AddComponent<Editor::EditTag>(debugCamera)->name = "DebugCamera";
	auto data = Editor::SaveScene(registry);
	data["systems"] = { "EditSystem", "DrawManipulatorSystem" };
	return SaveNewScene(name, data);
}

bool SceneManager::CopyScene(const std::string& name) {
	if (!currentScene_) return false;
	auto data = Editor::SaveScene(currentScene_->GetRegistry());
	data["systems"] = currentScene_->GetSystemNames();
	try {
		std::ifstream source(ECS::SystemSceneConfig::GetScenePath(currentScene_->GetName()));
		if (source) {
			nlohmann::json original;
			source >> original;
			if (original.contains("renderPasses")) data["renderPasses"] = original["renderPasses"];
		}
	} catch (const std::exception& error) {
		LogWarning(std::string("Failed to copy scene: ") + error.what());
		return false;
	}
	return SaveNewScene(name, data);
}

bool SceneManager::DeleteScene(const std::string& name) {
	if (!sceneNames_.contains(name) || transitionPhase_ != TransitionPhase::None) return false;
	auto names = sceneNames_;
	names.erase(name);
	// 描画側がRegistryを使い続けられるよう、最後のシーンは残す。
	if (names.empty()) return false;
	std::unique_ptr<Scene> fallback;
	if (currentScene_ && currentScene_->GetName() == name) {
		for (const auto& candidate : names) {
			fallback = LoadScene(candidate);
			if (fallback) break;
		}
		if (!fallback) return false;
	}
	if (!SaveSceneList(names)) return false;
	std::error_code error;
	std::filesystem::remove(ECS::SystemSceneConfig::GetScenePath(name), error);
	if (error) {
		SaveSceneList(sceneNames_);
		LogWarning("Failed to delete scene: " + error.message());
		return false;
	}
	sceneNames_ = std::move(names);
	if (fallback) CommitScene(std::move(fallback));
	return true;
}

void SceneManager::Update(ComputeContext& ctx, float deltaTime) {
	if (transitionPhase_ != TransitionPhase::None) {
		const float half = transitionDuration_ * 0.5f;
		if (deltaTime > 0.0f && deltaTime < 0.1f) transitionTimer_ += deltaTime;
		const float t = std::clamp(transitionTimer_ / half, 0.0f, 1.0f);
		if (transitionPhase_ == TransitionPhase::FadingOut) {
			if (currentEffect_) currentEffect_->UpdateFadingOut(currentScene_->GetRegistry(), t);
			if (transitionTimer_ >= half) {
				CommitScene(std::move(pendingScene_));
				if (currentEffect_) currentEffect_->OnFadingInStart(currentScene_->GetRegistry());
				transitionPhase_ = TransitionPhase::FadingIn;
				transitionTimer_ = 0.0f;
			}
		} else {
			if (currentEffect_) currentEffect_->UpdateFadingIn(currentScene_->GetRegistry(), t);
			if (transitionTimer_ >= half) ResetTransition();
		}
	}
	if (currentScene_) currentScene_->Update(ctx, deltaTime);
	// 更新中のSystemを破棄しないよう、Stopと再読み込みを更新終了後に処理する。
	if (auto restore = ECS::PlayModeController::TakeSceneRestoreRequest()) {
		// 同名シーンでは既にスナップショットへ復元済みなので、演出の古いEntity IDに触れない。
		ResetTransition(currentScene_ && currentScene_->GetName() != *restore);
		if (currentScene_ && currentScene_->GetName() != *restore) ChangeScene(*restore);
	} else if (currentScene_ && currentScene_->GetRegistry().PollEvent<Event::SceneReloadEvent>()) {
		ReloadScene();
	}
	DrawSceneMenu();
}

void SceneManager::DrawSceneMenu() {
#ifdef USE_IMGUI
	static bool requestCreate = false;
	static bool requestCopy = false;
	static bool requestDelete = false;
	static char sceneName[128] = {};
	static char copyName[128] = {};
	static std::string pendingDelete;
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Scene")) {
			if (ImGui::MenuItem("Create new scene...")) { sceneName[0] = '\0'; requestCreate = true; }
			if (ImGui::MenuItem("Copy current scene...")) { copyName[0] = '\0'; requestCopy = true; }
			for (const auto& name : sceneNames_) {
				if (ImGui::MenuItem(name.c_str()) && currentScene_) {
					Event::SceneChangeEvent event;
					event.nextScene = name;
					event.transitionType = Event::SceneTransitionType::kImmediate;
					GetRegistry()->EmitEvent(event);
				}
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Delete current scene...", nullptr, false, currentScene_ && sceneNames_.size() > 1)) { pendingDelete = currentScene_->GetName(); requestDelete = true; }

			ImGui::EndMenu();
		}
		// Fileはシーン更新中に描画済みなので、Sceneの後ろにWindowを追加する。
		Editor::EditorWindowManager::GetInstance().DrawMenu();
		ImGui::EndMainMenuBar();
	}
	// メニューバーを閉じてからモーダルポップアップを開いて描画し、ルートのポップアップスタックを使う
	if (requestCreate) { ImGui::OpenPopup("Create new scene"); requestCreate = false; }
	if (ImGui::BeginPopupModal("Create new scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::InputText("Name", sceneName, sizeof(sceneName));
		if (ImGui::Button("Create") && CreateScene(sceneName)) ImGui::CloseCurrentPopup();
		ImGui::SameLine(); if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup(); ImGui::EndPopup();
	}
	if (requestCopy) { ImGui::OpenPopup("Copy scene"); requestCopy = false; }
	if (ImGui::BeginPopupModal("Copy scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::InputText("Name", copyName, sizeof(copyName));
		if (ImGui::Button("Copy") && CopyScene(copyName)) ImGui::CloseCurrentPopup();
		ImGui::SameLine(); if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup(); ImGui::EndPopup();
	}
	if (requestDelete) { ImGui::OpenPopup("Delete scene"); requestDelete = false; }
	if (ImGui::BeginPopupModal("Delete scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Delete '%s'?", pendingDelete.c_str());
		if (ImGui::Button("Delete") && DeleteScene(pendingDelete)) ImGui::CloseCurrentPopup();
		ImGui::SameLine(); if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup(); ImGui::EndPopup();
	}
#endif // USE_IMGUI

}
}
}
