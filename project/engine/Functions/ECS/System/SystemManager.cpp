#include "SystemManager.h"
#include "../Component/Common/PauseComponent.h"
#include "../Event/SceneChangeEvent.h"
#include "../../Scene/SceneNameComponent.h"
#include "../../Scene/IScene.h"
#include "../../Particle/ParticleManager.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"
#include "engine/Editor/EditTag.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

#include <unordered_set>
#include <algorithm>
#include <fstream>
#include <map>

namespace NoEngine {
namespace ECS {

namespace {
struct SystemCatalog {
	std::unordered_map<std::string, SystemManager::SystemRegistration> factories;
	std::unordered_map<std::type_index, std::string> types;
};

SystemCatalog& GetSystemCatalog() {
	static SystemCatalog catalog;
	return catalog;
}

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
std::string startSceneName = "";
bool changedSceneInPlaying = false;
}

bool SystemManager::RegisterSystemType(const std::string& name, const std::string& category,
	std::type_index type, SystemFactory factory) {
	auto& catalog = GetSystemCatalog();
	catalog.factories[name] = SystemRegistration{ category, std::move(factory) };
	catalog.types[type] = name;
	return true;
}

SystemManager::SystemManager() {
	auto& catalog = GetSystemCatalog();
	systemFactories_ = catalog.factories;
	systemTypes_ = catalog.types;
}

bool SystemManager::AddSystemByName(const std::string& name) {
	auto factory = systemFactories_.find(name);
	if (factory == systemFactories_.end()) return false;
	systems_.push_back(factory->second.factory());
	systemNames_.push_back(name);
	return true;
}

void SystemManager::RemoveSystem(size_t index) {
	if (index >= systems_.size()) return;
	systems_.erase(systems_.begin() + index);
	systemNames_.erase(systemNames_.begin() + index);
}

void SystemManager::MoveSystem(size_t from, size_t to) {
	if (from >= systems_.size() || to >= systems_.size() || from == to) return;
	auto system = std::move(systems_[from]);
	auto name = std::move(systemNames_[from]);
	systems_.erase(systems_.begin() + from);
	systemNames_.erase(systemNames_.begin() + from);
	systems_.insert(systems_.begin() + to, std::move(system));
	systemNames_.insert(systemNames_.begin() + to, std::move(name));
}

void SystemManager::LoadSystemConfiguration(Registry& registry) {
	std::string sceneName = Scene::GetCurrentSceneName(registry);
	if (sceneName.empty() || sceneName == configuredSceneName_) return;
	configuredSceneName_ = sceneName;
	std::ifstream file("resources/game/Scenes/" + sceneName + ".json");
	if (!file) return;
	try {
		nlohmann::json data; file >> data;
		if (!data.contains("systems") || !data["systems"].is_array()) return;
		std::vector<std::unique_ptr<ISystem>> configured;
		std::vector<std::string> names;
		for (const auto& value : data["systems"]) {
			if (!value.is_string()) return;
			const auto name = value.get<std::string>();
			auto factory = systemFactories_.find(name);
			if (factory == systemFactories_.end()) return;
			names.push_back(name);
			configured.push_back(factory->second.factory());
		}
		systems_ = std::move(configured);
		systemNames_ = std::move(names);
	} catch (const std::exception&) {
		LogWarning("Failed to load scene system configuration: " + sceneName);
	}
}

void SystemManager::SaveSystemConfiguration(Registry& registry) {
	const auto sceneName = Scene::GetCurrentSceneName(registry);
	if (sceneName.empty() || sceneName.find_first_of("/\\:") != std::string::npos) return;
	const std::string path = "resources/game/Scenes/" + sceneName + ".json";
	std::ifstream in(path);
	nlohmann::json data = nlohmann::json::object();
	if (in) { try { in >> data; } catch (const std::exception&) { return; } }
	data["systems"] = systemNames_;
	std::ofstream out(path);
	if (out) out << data.dump(4);
}

bool SystemManager::IsInPlayMode() {
	return sPlayState != EditorPlayState::kEditing;
}

const nlohmann::json& SystemManager::GetPlaySnapshot() {
	return sPlaySnapshot;
}

void SystemManager::LoadPlaySnapShot(Registry& registry) {
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
}

void SystemManager::UpdateAll(ComputeContext& ctx, Registry& registry, float deltaTime) {
	LoadSystemConfiguration(registry);
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

	// プレイ中にシーンが切り替わった場合のスナップショット読み込み
	if (changedSceneInPlaying) {
		LoadPlaySnapShot(registry);
		changedSceneInPlaying = false;
	}

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
			// 再生中にシーンが切り替わったら元のシーンに戻す
			for (auto e : registry.View<SceneNameComponent>()) {
				auto* sceneName = registry.GetComponent<SceneNameComponent>(e);
				if (startSceneName != sceneName->GetName()) {
					Event::SceneChangeEvent event;
					event.nextScene = startSceneName;
					event.transitionType = Event::SceneTransitionType::kImmediate;
					registry.EmitEvent(event);
					changedSceneInPlaying = true;
				}
			}

			if (!changedSceneInPlaying) {
				LoadPlaySnapShot(registry);
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
			for (auto e : registry.View<SceneNameComponent>()) startSceneName = registry.GetComponent<SceneNameComponent>(e)->GetName();
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

	ImGui::Begin("Systems");
	if (ImGui::BeginCombo("Add system", "Select system...")) {
		std::map<std::string, std::vector<std::string>> categories;
		for (const auto& [name, registration] : systemFactories_) categories[registration.category].push_back(name);
		for (auto& [category, names] : categories) {
			std::sort(names.begin(), names.end());
			ImGui::TextDisabled("%s", category.c_str());
			for (const auto& name : names) {
				bool alreadyAdded = std::find(systemNames_.begin(), systemNames_.end(), name) != systemNames_.end();
				ImGui::Indent();
				if (ImGui::Selectable(name.c_str(), false, alreadyAdded ? ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None) && !alreadyAdded) {
					if (AddSystemByName(name)) SaveSystemConfiguration(registry);
				}
				ImGui::Unindent();
			}
		}
		ImGui::EndCombo();
	}
	for (size_t i = 0; i < systemNames_.size(); ++i) {
		ImGui::PushID(static_cast<int>(i));
		bool reordered = false;
		ImGui::Text("%zu. %s", i + 1, systemNames_[i].c_str());
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
			ImGui::SetDragDropPayload("SYSTEM_INDEX", &i, sizeof(i));
			ImGui::Text("Move %s", systemNames_[i].c_str());
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SYSTEM_INDEX")) {
				const size_t from = *static_cast<const size_t*>(payload->Data);
				if (payload->IsDelivery() && from < systemNames_.size() && from != i) {
					const float midpoint = (ImGui::GetItemRectMin().y + ImGui::GetItemRectMax().y) * 0.5f;
					const bool dropAfter = ImGui::GetMousePos().y >= midpoint;
					size_t to = i;
					if (dropAfter && from > i) ++to;
					else if (!dropAfter && from < i) --to;
					MoveSystem(from, to);
					SaveSystemConfiguration(registry);
					reordered = true;
				}
			}
			ImGui::EndDragDropTarget();
		}
		if (reordered) { ImGui::PopID(); break; }
		if (ImGui::BeginPopupContextItem("SystemContext")) {
			if (ImGui::MenuItem("Remove")) {
				RemoveSystem(i);
				SaveSystemConfiguration(registry);
				ImGui::EndPopup();
				ImGui::PopID();
				break;
			}
			ImGui::EndPopup();
		}
		ImGui::PopID();
	}
	ImGui::End();
#endif // USE_IMGUI

}
}
}
