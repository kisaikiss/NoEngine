#include "PrefabEditorPanel.h"
#include "engine/Editor/Components/ComponentUI.h"
#include "engine/Editor/DataDriven/PrefabSerializer.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"
#include "engine/Editor/Panels/InspectorPanel.h"
#include "engine/Editor/Commands/EditorCommandOperator.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

#include <fstream>
#include <chrono>
#include <system_error>

namespace NoEngine {
namespace Editor {

void PrefabEditorPanel::Begin(const std::string& prefabPath) {
	if (IsEditing()) {
		if (prefabPath == prefabPath_ || pendingAction_ != PendingAction::None) return;
		pendingAction_ = PendingAction::Switch;
		pendingPath_ = prefabPath;
		return;
	}
	Open(prefabPath);
}

void PrefabEditorPanel::Open(const std::string& prefabPath) {
	try {
		// 読み込みに成功してから切り替え、シーンのRegistryにはEntityを生成しない。
		auto editingRegistry = std::make_shared<ECS::Registry>();
		const auto entity = Editor::LoadPrefabForEditing(*editingRegistry, prefabPath);
		auto savedJson = Editor::SaveEntityToJson(*editingRegistry, entity);
		Clear();
		editingRegistry_ = std::move(editingRegistry);
		prefabEntity_ = entity;
		prefabPath_ = prefabPath;
		savedJson_ = std::move(savedJson);
	} catch (const std::exception& error) {
		errorMessage_ = std::string("Prefabの読み込みに失敗しました: ") + error.what();
		LogError(errorMessage_);
	}
}

void PrefabEditorPanel::End() {
	if (IsEditing() && pendingAction_ == PendingAction::None) pendingAction_ = PendingAction::Close;
}

bool PrefabEditorPanel::HasUnsavedChanges() const {
	return IsEditing() && Editor::SaveEntityToJson(*editingRegistry_, prefabEntity_) != savedJson_;
}

void PrefabEditorPanel::Clear() {
	editingRegistry_.reset();
	prefabEntity_ = ECS::INVALID_ENTITY;
	prefabPath_.clear();
	savedJson_ = {};
	pendingAction_ = PendingAction::None;
	pendingPath_.clear();
	errorMessage_.clear();
}

void PrefabEditorPanel::CompletePendingAction() {
	const auto action = pendingAction_;
	const auto path = pendingPath_;
	pendingAction_ = PendingAction::None;
	pendingPath_.clear();
	if (action == PendingAction::Close) Clear();
	else if (action == PendingAction::Switch) Open(path);
}

bool PrefabEditorPanel::SaveAndApply(ECS::Registry& sceneRegistry) {
	std::filesystem::path temporaryPath;
	try {
		auto prefabJson = Editor::SaveEntityToJson(*editingRegistry_, prefabEntity_);
		const std::filesystem::path target(prefabPath_);
		temporaryPath = target;
		temporaryPath += "." + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".tmp";
		// 書き込みを完了してから置き換え、失敗時に元のPrefabを壊さない。
		std::ofstream output;
		output.exceptions(std::ios::failbit | std::ios::badbit);
		output.open(temporaryPath, std::ios::binary | std::ios::trunc);
		output << prefabJson.dump(4);
		output.close();
		if (!MoveFileExW(temporaryPath.c_str(), target.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
			throw std::system_error(static_cast<int>(GetLastError()), std::system_category());
		}
		Editor::ApplyPrefabToInstances(sceneRegistry, prefabPath_, prefabJson);
		savedJson_ = std::move(prefabJson);
		errorMessage_.clear();
	} catch (const std::exception& error) {
		std::error_code ignored;
		if (!temporaryPath.empty()) std::filesystem::remove(temporaryPath, ignored);
		errorMessage_ = std::string("Prefabの保存または反映に失敗しました: ") + error.what();
		LogError(errorMessage_);
		return false;
	}
	try {
		Editor::LoadPrefabsFromDirectory();
	} catch (const std::exception& error) {
		LogWarning(std::string("Prefab一覧の更新に失敗しました: ") + error.what());
	}
	return true;
}

void PrefabEditorPanel::Draw(ECS::Registry& sceneRegistry) {
#ifdef USE_IMGUI
	if (!IsEditing() && errorMessage_.empty()) return;

	bool open = true;
	ImGui::Begin("Prefab Editor", &open, HasUnsavedChanges() ? ImGuiWindowFlags_UnsavedDocument : 0);
	if (!IsEditing()) {
		ImGui::TextWrapped("%s", errorMessage_.c_str());
		if (!open || ImGui::Button("Close")) errorMessage_.clear();
		ImGui::End();
		return;
	}
	ImGui::TextDisabled("%s", prefabPath_.c_str());
	ImGui::Separator();

	{
		ScopedEditorRegistry scope(editingRegistry_);
		DrawComponentUI(*editingRegistry_, prefabEntity_);
		InspectorPanel::DrawAddComponentMenu(*editingRegistry_, prefabEntity_);
	}
	if (HasUnsavedChanges()) ImGui::TextUnformatted("未保存の変更があります");
	ImGui::Separator();
	if (ImGui::Button("Save & Apply")) {
		SaveAndApply(sceneRegistry);
	}
	ImGui::SameLine();
	if (ImGui::Button("Close") || !open) {
		End();
	}
	if (!errorMessage_.empty()) ImGui::TextWrapped("%s", errorMessage_.c_str());

	// UIの編集確定後に判定することで、切り替えクリック時の入力も保護する。
	if (pendingAction_ != PendingAction::None && !ImGui::IsPopupOpen("未保存の変更###PrefabUnsavedChanges")) {
		if (!HasUnsavedChanges()) {
			CompletePendingAction();
		} else {
			ImGui::OpenPopup("未保存の変更###PrefabUnsavedChanges");
		}
	}
	if (ImGui::BeginPopupModal("未保存の変更###PrefabUnsavedChanges", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::TextWrapped("%s に未保存の変更があります。", prefabPath_.c_str());
		ImGui::TextUnformatted("保存して反映してから続行しますか？");
		if (!errorMessage_.empty()) ImGui::TextWrapped("%s", errorMessage_.c_str());
		if (ImGui::Button("保存")) {
			if (SaveAndApply(sceneRegistry)) {
				ImGui::CloseCurrentPopup();
				CompletePendingAction();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("破棄")) {
			ImGui::CloseCurrentPopup();
			CompletePendingAction();
		}
		ImGui::SameLine();
		if (ImGui::Button("キャンセル")) {
			pendingAction_ = PendingAction::None;
			pendingPath_.clear();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::End();
#else
	static_cast<void>(sceneRegistry);
#endif // USE_IMGUI
}

}
}
