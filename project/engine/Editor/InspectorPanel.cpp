#include "InspectorPanel.h"
#include "engine/Editor/DataDriven/PrefabSerializer.h"
#include "engine/Editor/ComponentUI.h"
#include "engine/Editor/ReflectionMacros.h"
#include "engine/Editor/EditorCommandOperator.h"
#include "engine/Functions/Command/EditCommand/AddComponentCommand.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

#include <map>
#include <algorithm>
#include <cctype>

namespace NoEngine {
namespace Editor {

void InspectorPanel::Draw(ECS::Registry& registry) {
#ifdef USE_IMGUI
	ImGui::Begin("Inspector");

	auto selectedView = registry.View<Editor::EditSelectedTag, Editor::EditTag>();
	for (auto e : selectedView) {
		auto* tag = registry.GetComponent<Editor::EditTag>(e);
		if (tag) {
			ImGui::BeginChild(tag->name.c_str());
			ImGui::Text(tag->name.c_str());
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
		addComponentFilter_[0] = '\0';
	}

	if (ImGui::BeginPopup("AddComponentPopup")) {

		ImGui::SetNextItemWidth(200.0f);
		ImGui::InputTextWithHint("##AddComponentFilter", "Search...", addComponentFilter_, sizeof(addComponentFilter_));
		ImGui::Separator();

		std::string filter = addComponentFilter_;
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
