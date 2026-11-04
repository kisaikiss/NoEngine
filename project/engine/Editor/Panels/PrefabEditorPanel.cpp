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

namespace NoEngine {
namespace Editor {
namespace {
nlohmann::json ReadDefinitionFile(const std::string& path) {
	std::ifstream input(path);
	if (!input) throw std::runtime_error("Prefabを開けません: " + path);
	nlohmann::json result;
	input >> result;
	return result;
}
}

void PrefabEditorPanel::Begin(const std::string& prefabPath) {
	if (IsEditing()) {
		if (prefabPath == prefabPath_ || pendingAction_ != PendingAction::None) return;
		pendingAction_ = PendingAction::Switch;
		pendingPath_ = prefabPath;
		return;
	}
	Open(prefabPath);
}

void PrefabEditorPanel::Open(std::string prefabPath) {
	try {
		// 読み込みに成功してから切り替え、シーンのRegistryにはEntityを生成しない。
		auto editingRegistry = std::make_shared<ECS::Registry>();
		const auto entity = Editor::LoadPrefabForEditing(*editingRegistry, prefabPath);
		auto savedJson = Editor::SaveEntityToJson(*editingRegistry, entity);
		auto diskJson = ReadDefinitionFile(prefabPath);
		Clear();
		editingRegistry_ = std::move(editingRegistry);
		prefabEntity_ = entity;
		prefabPath_ = prefabPath;
		savedJson_ = std::move(savedJson);
		diskJson_ = std::move(diskJson);
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
	diskJson_ = {};
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
	try {
		auto prefabJson = Editor::SaveEntityToJson(*editingRegistry_, prefabEntity_);
		if (ReadDefinitionFile(prefabPath_) != diskJson_) {
			throw std::runtime_error("別の操作でPrefabが更新されています。現在の変更を確認し、破棄して開き直してから編集してください。");
		}
		SavePrefabFile(prefabPath_, prefabJson);
		diskJson_ = ReadDefinitionFile(prefabPath_);
		Editor::ApplyPrefabToInstances(sceneRegistry, prefabPath_, prefabJson);
		savedJson_ = std::move(prefabJson);
		errorMessage_.clear();
	} catch (const std::exception& error) {
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
	// InspectorのApplyでファイルが変わった場合、未編集の画面だけ最新内容に切り替える。
	if (pendingAction_ == PendingAction::None && !HasUnsavedChanges() && !ImGui::IsAnyItemActive()) {
		try {
			if (ReadDefinitionFile(prefabPath_) != diskJson_) Open(prefabPath_);
		} catch (const std::exception& error) {
			errorMessage_ = std::string("Prefabの更新確認に失敗しました: ") + error.what();
		}
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
