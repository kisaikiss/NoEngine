#include "SceneManager.h"

#include "CircleScaleTransitionEffect.h"
#include "FadeTransitionEffect.h"
#include "engine/Functions/ECS/System/Editor/EditSystem.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"
#include <fstream>
#include <filesystem>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

namespace NoEngine {
namespace Scene {
namespace {
const char* kSceneDirectory = "resources/game/Scenes/";
const char* kSceneListPath = "resources/game/application.json";

class EditorScene final : public IScene {
public:
	void Setup() override { AddSystem(std::make_unique<ECS::EditSystem>()); }
};

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
			for (const auto& name : names.value("removedScenes", std::vector<std::string>{}))
				if (IsValidSceneName(name)) removedScenes_.insert(name);
			for (const auto& name : names.value("scenes", std::vector<std::string>{})) {
				if (IsValidSceneName(name) && factories_.find(name) == factories_.end() && std::filesystem::exists(std::string(kSceneDirectory) + name + ".json")) {
					RegisterScene(name, [] { return std::make_unique<EditorScene>(); });
					editorScenes_.insert(name);
				}
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

void SceneManager::ChangeScene(const std::string& name, bool immediate,
	Event::SceneTransitionType transitionType, const std::string& customTransitionName) {
	auto it = factories_.find(name);
	if (it == factories_.end()) return;
	if (isChanging_) return;
	isChanging_ = true;

	if (immediate || !currentScene_) {
		if (currentScene_) currentScene_->OnExit();
		currentScene_ = it->second();
		currentScene_->SetName(name);
		currentScene_->Setup();
		currentScene_->OnEnter();

		isTransitioning_ = false;
		transitionPhase_ = TransitionPhase::None;
		transitionTimer_ = 0.0f;
		currentEffect_.reset();
		isChanging_ = false;
		return;
	}

	pendingName_ = name;
	isTransitioning_ = true;
	transitionPhase_ = TransitionPhase::FadingOut;
	transitionTimer_ = 0.0f;

	currentEffect_ = CreateTransitionEffect(transitionType, customTransitionName);
	if (currentEffect_ && currentScene_->GetRegistry()) {
		currentEffect_->OnFadingOutStart(*currentScene_->GetRegistry());
	}
}

void SceneManager::Update(ComputeContext& ctx, float deltaTime) {

	if (isTransitioning_) {
		float half = transitionDuration_ * 0.5f;
		if (deltaTime > 0.0f && deltaTime < 0.1f) transitionTimer_ += deltaTime;

		if (transitionPhase_ == TransitionPhase::FadingOut) {
			float t = std::clamp(transitionTimer_ / half, 0.0f, 1.0f);
			if (currentEffect_ && currentScene_->GetRegistry())
				currentEffect_->UpdateFadingOut(*currentScene_->GetRegistry(), t);

			if (transitionTimer_ >= half) {
				if (currentScene_) currentScene_->OnExit();
				auto it = factories_.find(pendingName_);
				if (it != factories_.end()) {
					currentScene_ = it->second();
					currentScene_->SetName(pendingName_);
					currentScene_->Setup();
					currentScene_->OnEnter();
				}
				if (currentEffect_ && currentScene_->GetRegistry())
					currentEffect_->OnFadingInStart(*currentScene_->GetRegistry());

				transitionPhase_ = TransitionPhase::FadingIn;
				transitionTimer_ = 0.0f;
			}
		} else if (transitionPhase_ == TransitionPhase::FadingIn) {
			float t = std::clamp(transitionTimer_ / half, 0.0f, 1.0f);
			if (currentEffect_ && currentScene_->GetRegistry())
				currentEffect_->UpdateFadingIn(*currentScene_->GetRegistry(), t);

			if (transitionTimer_ >= half) {
				if (currentEffect_ && currentScene_->GetRegistry())
					currentEffect_->OnFinished(*currentScene_->GetRegistry());
				currentEffect_.reset();

				isTransitioning_ = false;
				transitionPhase_ = TransitionPhase::None;
				transitionTimer_ = 0.0f;
				isChanging_ = false;
			}
		}

	}

	if (currentScene_) currentScene_->Update(ctx, deltaTime);

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
			for (auto& factory : factories_) {
				if (factory.first == "") continue;
				if (ImGui::MenuItem(factory.first.c_str())) {
					Event::SceneChangeEvent event;
					event.nextScene = factory.first;
					event.transitionType = Event::SceneTransitionType::kImmediate;
					GetRegistry()->EmitEvent(event);
				}
			}
			ImGui::Separator();
			if (currentScene_ && ImGui::MenuItem("Delete current scene...")) { pendingDelete = GetCurrentSceneName(*currentScene_->GetRegistry()); requestDelete = true; }
			// Persist the app's scene catalog after creation/copy as well as deletion.
			static size_t knownCount = editorScenes_.size();
			if (knownCount != editorScenes_.size()) {
				nlohmann::json list; list["scenes"] = nlohmann::json::array(); for (const auto& item : editorScenes_) list["scenes"].push_back(item);
				list["removedScenes"] = nlohmann::json::array(); for (const auto& item : removedScenes_) list["removedScenes"].push_back(item);
				std::ofstream out(kSceneListPath); out << list.dump(4); knownCount = editorScenes_.size();
			}

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	// Open and draw modal popups after closing the menu bar so they use the root popup stack.
	if (requestCreate) { ImGui::OpenPopup("Create new scene"); requestCreate = false; }
	if (ImGui::BeginPopupModal("Create new scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::InputText("Name", sceneName, sizeof(sceneName));
		if (ImGui::Button("Create") && IsValidSceneName(sceneName) && factories_.find(sceneName) == factories_.end() && !std::filesystem::exists(std::string(kSceneDirectory) + sceneName + ".json")) {
			std::filesystem::create_directories(kSceneDirectory);
			std::ofstream out(std::string(kSceneDirectory) + sceneName + ".json"); out << R"({"entities":{}})";
			if (out) { removedScenes_.erase(sceneName); RegisterScene(sceneName, [] { return std::make_unique<EditorScene>(); }); editorScenes_.insert(sceneName); ChangeScene(sceneName); }
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine(); if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup(); ImGui::EndPopup();
	}
	if (requestCopy) { ImGui::OpenPopup("Copy scene"); requestCopy = false; }
	if (ImGui::BeginPopupModal("Copy scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::InputText("Name", copyName, sizeof(copyName));
		if (ImGui::Button("Copy") && IsValidSceneName(copyName) && factories_.find(copyName) == factories_.end() && !std::filesystem::exists(std::string(kSceneDirectory) + copyName + ".json") && currentScene_) {
			auto data = Editor::SaveScene(*currentScene_->GetRegistry());
			std::ofstream out(std::string(kSceneDirectory) + copyName + ".json"); out << data.dump(4);
			if (out) { removedScenes_.erase(copyName); RegisterScene(copyName, [] { return std::make_unique<EditorScene>(); }); editorScenes_.insert(copyName); ChangeScene(copyName); }
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine(); if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup(); ImGui::EndPopup();
	}
	if (requestDelete) { ImGui::OpenPopup("Delete scene"); requestDelete = false; }
	if (ImGui::BeginPopupModal("Delete scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Delete '%s'?", pendingDelete.c_str());
		if (ImGui::Button("Delete") && factories_.find(pendingDelete) != factories_.end()) {
			std::string fallback;
			for (const auto& item : factories_) if (item.first != pendingDelete) { fallback = item.first; break; }
			if (editorScenes_.count(pendingDelete)) {
				std::error_code ec; std::filesystem::remove(std::string(kSceneDirectory) + pendingDelete + ".json", ec);
				editorScenes_.erase(pendingDelete);
			} else removedScenes_.insert(pendingDelete);
			factories_.erase(pendingDelete);
			nlohmann::json list; list["scenes"] = nlohmann::json::array(); for (const auto& item : editorScenes_) list["scenes"].push_back(item);
			list["removedScenes"] = nlohmann::json::array(); for (const auto& item : removedScenes_) list["removedScenes"].push_back(item);
			std::ofstream out(kSceneListPath); out << list.dump(4);
			if (!fallback.empty()) ChangeScene(fallback);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine(); if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup(); ImGui::EndPopup();
	}
#endif // USE_IMGUI

}
}
}
