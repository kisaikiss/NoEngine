#include "InspectorPanel.h"
#include "engine/Editor/Utilities/MultiEdit.h"
#include "engine/Editor/DataDriven/PrefabSerializer.h"
#include "engine/Editor/Components/ComponentUI.h"
#include "engine/Editor/Commands/EditorCommandOperator.h"
#include "engine/Functions/Command/EditCommand/AddComponentCommand.h"
#include "engine/Functions/Command/EditCommand/FunctionCommand.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI
 
#include <cctype>

namespace {

char sAddComponentFilter[128] = {};
#ifdef USE_IMGUI
void DrawPrefabOverrides(NoEngine::ECS::Registry& registry, NoEngine::ECS::Entity entity) {
	using namespace NoEngine;
	using namespace NoEngine::Editor;
	const auto* instance = registry.GetComponent<PrefabInstanceTag>(entity);
	if (!instance) return;
	static std::string errorMessage;
	static ECS::Registry* errorRegistry = nullptr;
	static ECS::Entity errorEntity = ECS::INVALID_ENTITY;
	if (errorRegistry != &registry || errorEntity != entity) errorMessage.clear();
	errorRegistry = &registry;
	errorEntity = entity;
	ImGui::PushID(static_cast<int>(entity));
	ImGui::TextWrapped("Prefab: %s", instance->prefabPath.c_str());
	try {
		const auto overrides = GetPrefabOverrides(registry, entity);
		ImGui::Text("Overrides: %zu", overrides.size());
		auto revert = [&](const std::string& path) {
			const auto before = SaveEntityToJson(registry, entity);
			RevertPrefabOverrides(registry, entity, path);
			const auto after = SaveEntityToJson(registry, entity);
			EditorCommandOperator::AddCommand(std::make_unique<Command::FunctionCommand>(
				[&registry, entity, after] { if (registry.Has<EditTag>(entity)) LoadEntityFromJson(registry, entity, after, true); },
				[&registry, entity, before] { if (registry.Has<EditTag>(entity)) LoadEntityFromJson(registry, entity, before, true); }, "Revert prefab overrides"));
		};
		ImGui::BeginDisabled(overrides.empty());
		int action = 0;
		std::string selectedPath;
		if (ImGui::Button("Revert All")) action = 1;
		ImGui::SameLine();
		if (ImGui::Button("Apply All")) action = 2;
		if (!action && ImGui::TreeNode("Overrides一覧")) {
			for (const auto& entry : overrides) {
				const std::string path = entry.at("path");
				ImGui::PushID(path.c_str());
				std::string label = path.substr(std::string("/components/").size());
				std::replace(label.begin(), label.end(), '/', '.');
				const auto operation = entry.at("op").get<std::string>();
				ImGui::TextWrapped("%s (%s)", label.c_str(), operation == "remove" ? "削除" : operation == "add" ? "追加" : "変更");
				const nlohmann::json::json_pointer pointer(path);
				if (instance->base.contains(pointer)) ImGui::TextWrapped("元: %s", instance->base[pointer].dump().c_str());
				if (entry.contains("value")) ImGui::TextWrapped("現在: %s", entry.at("value").dump().c_str());
				if (ImGui::SmallButton("Revert")) { selectedPath = path; action = 1; }
				ImGui::SameLine();
				if (ImGui::SmallButton("Apply")) { selectedPath = path; action = 2; }
				ImGui::PopID();
				if (action) break;
			}
			ImGui::TreePop();
		}
		ImGui::EndDisabled();
		// UIのスタックを閉じてから、失敗する可能性があるファイル操作を行う。
		if (action == 1) revert(selectedPath);
		if (action == 2) ApplyInstanceOverrides(registry, entity, selectedPath);
		if (action) errorMessage.clear();
	} catch (const std::exception& error) {
		// ファイルが消失した場合も、シーン上の編集値はそのまま保持する。
		errorMessage = std::string("Prefab操作に失敗しました: ") + error.what();
	}
	if (!errorMessage.empty()) ImGui::TextWrapped("%s", errorMessage.c_str());
	ImGui::Separator();
	ImGui::PopID();
}
#endif
}

namespace NoEngine {
namespace Editor {

void InspectorPanel::Draw(ECS::Registry& registry) {
#ifdef USE_IMGUI
	ImGui::Begin("Inspector");
	const auto selection = GetEditorSelection(registry);
	DrawMultiInspector(registry, selection);
	if (selection.size() > 1) {
		ImGui::End();
		return;
	}

	auto selectedView = registry.View<Editor::EditSelectedTag, Editor::EditTag>();
	for (auto e : selectedView) {
		auto* tag = registry.GetComponent<Editor::EditTag>(e);
		if (tag) {
			ImGui::BeginChild(tag->name.c_str());
			ImGui::Text(tag->name.c_str());
			DrawPrefabOverrides(registry, e);
			DrawComponentUI(registry, e);
			if (ImGui::Button("SavePreset")) {
				Editor::SavePreset(registry, e);
			}
			DrawAddComponentMenu(registry, e);
			ImGui::EndChild();
		}
	}

	ImGui::End();
#else
	static_cast<void>(registry);
#endif // USE_IMGUI
}

void InspectorPanel::DrawAddComponentMenu(ECS::Registry& registry, ECS::Entity entity) {
#ifdef USE_IMGUI
	if (ImGui::Button("Add Component")) {
		ImGui::OpenPopup("AddComponentPopup");
		sAddComponentFilter[0] = '\0';
	}

	if (ImGui::BeginPopup("AddComponentPopup")) {

		ImGui::SetNextItemWidth(200.0f);
		ImGui::InputTextWithHint("##AddComponentFilter", "Search...", sAddComponentFilter, sizeof(sAddComponentFilter));
		ImGui::Separator();

		std::string filter = sAddComponentFilter;
		std::transform(filter.begin(), filter.end(), filter.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		const bool isFiltering = !filter.empty();

		// カテゴリごとに束ねる（Registry の登録順を保ちたいので vector で保持）
		std::map<std::string, std::vector<const NoEngine::TypeInfo*>> grouped;

		for (auto& typeInfo : NoEngine::ComponentRegistry::GetAll()) {

			// すでに持っている Component はスキップ
			if (registry.Has(typeInfo.typeId, entity))
				continue;

			if (isFiltering) {
				std::string lowerName = typeInfo.name;
				std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
					[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
				if (lowerName.find(filter) == std::string::npos)
					continue;
			}

			grouped[typeInfo.category].push_back(&typeInfo);
		}

		auto addComponentItem = [&](const NoEngine::TypeInfo* typeInfo) {
			if (ImGui::MenuItem(typeInfo->name.c_str())) {
				// adder を使って Component を追加
				typeInfo->adder(registry, entity);
				Editor::EditorCommandOperator::AddCommand(std::make_unique<Command::AddComponentCommand>(registry, entity, typeInfo->typeId));
			}
			};

		for (auto& [category, typeInfos] : grouped) {
			if (isFiltering) {
				// 検索中はカテゴリを畳まず、見出しとして表示するだけにする
				ImGui::TextDisabled("%s", category.c_str());
				for (auto* typeInfo : typeInfos) {
					addComponentItem(typeInfo);
				}
			} else if (ImGui::BeginMenu(category.c_str())) {
				for (auto* typeInfo : typeInfos) {
					addComponentItem(typeInfo);
				}
				ImGui::EndMenu();
			}
		}

		ImGui::EndPopup();
	}

#else
	static_cast<void>(registry);
	static_cast<void>(entity);
#endif // USE_IMGUI
}

}
}
