#include "stdafx.h"
#include "SystemListPanel.h"
#include "EditorWindowManager.h"
#include "engine/Editor/Commands/EditorCommandOperator.h"
#include "engine/Functions/ECS/System/SystemManager.h"
#include "engine/Functions/ECS/System/SystemCatalog.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

namespace NoEngine {
namespace Editor {

#ifdef USE_IMGUI
namespace {

struct SystemDragPayload {
	const ECS::SystemManager* manager;
	size_t index;
};

// 行の上下半分を挿入境界に変換し、確定前から挿入線を表示する。
bool AcceptReorderDrop(const ECS::SystemManager& manager, size_t targetIndex, size_t count,
	bool atEnd, size_t& outFrom, size_t& outTo) {
	const ImVec2 rectMin = ImGui::GetItemRectMin();
	const ImVec2 rectMax = ImGui::GetItemRectMax();
	bool accepted = false;
	if (ImGui::BeginDragDropTarget()) {
		const auto flags = ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect;
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SYSTEM_ROW", flags)) {
			if (payload->DataSize == sizeof(SystemDragPayload)) {
				const auto& source = *static_cast<const SystemDragPayload*>(payload->Data);
				if (source.manager == &manager && source.index < count) {
					const bool after = !atEnd && ImGui::GetMousePos().y >= (rectMin.y + rectMax.y) * 0.5f;
					const size_t slot = atEnd ? count : targetIndex + (after ? 1 : 0);
					const size_t to = slot > source.index ? slot - 1 : slot;
					if (to != source.index) {
						const float y = after ? rectMax.y : rectMin.y;
						const ImU32 color = ImGui::GetColorU32(ImGuiCol_DragDropTarget);
						auto* drawList = ImGui::GetWindowDrawList();
						drawList->AddLine(ImVec2(rectMin.x, y), ImVec2(rectMax.x, y), color, 3.0f);
						drawList->AddCircleFilled(ImVec2(rectMin.x + 3.0f, y), 4.0f, color);
						if (payload->IsDelivery()) {
							outFrom = source.index;
							outTo = to;
							accepted = true;
						}
					}
				}
			}
		}
		ImGui::EndDragDropTarget();
	}
	return accepted;
}

}
#endif // USE_IMGUI

bool SystemListPanel::Draw(ECS::SystemManager& manager) {
	bool changed = false;
#ifdef USE_IMGUI
	auto& state = EditorWindowManager::GetInstance().Get(EditorWindowId::Systems);
	if (!state.open) return false;
	if (ImGui::Begin(state.label, &state.open)) {
		ImGui::BeginDisabled(!EditorCommandOperator::CanUndo());
		if (ImGui::Button("Undo")) EditorCommandOperator::Undo();
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Undo last editor action (Ctrl+Z)");
		ImGui::SameLine();
		ImGui::BeginDisabled(!EditorCommandOperator::CanRedo());
		if (ImGui::Button("Redo")) EditorCommandOperator::Redo();
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Redo last editor action (Ctrl+Shift+Z)");

		if (selectedIndex_ >= static_cast<int>(manager.GetSystemNames().size())) selectedIndex_ = -1;
		changed |= DrawAddCombo(manager);
		const size_t count = manager.GetSystemNames().size();
		if (selectedIndex_ >= 0)
			ImGui::TextDisabled("%zu systems | Selected: %d", count, selectedIndex_ + 1);
		else
			ImGui::TextDisabled("%zu systems | Select a row, drag to reorder", count);
		ImGui::Separator();
		changed |= DrawSystemList(manager);
	}
	ImGui::End();
#else
	static_cast<void>(manager);
#endif // USE_IMGUI
	return changed;
}

bool SystemListPanel::DrawAddMenu(ECS::SystemManager& manager, size_t insertionIndex) {
	bool changed = false;
#ifdef USE_IMGUI
	for (const auto& [category, names] : ECS::SystemCatalog::Get().GroupedByCategory()) {
		if (ImGui::BeginMenu(category.c_str())) {
			for (const auto& name : names) {
				if (ImGui::MenuItem(name.c_str(), nullptr, false, !manager.HasSystem(name))) {
					changed = manager.EditAddSystem(name, insertionIndex);
				}
			}
			ImGui::EndMenu();
		}
	}
#else
	static_cast<void>(manager);
	static_cast<void>(insertionIndex);
#endif // USE_IMGUI
	return changed;
}

bool SystemListPanel::DrawAddCombo(ECS::SystemManager& manager) {
	bool changed = false;
#ifdef USE_IMGUI
	if (ImGui::BeginCombo("Add system", "Choose insertion position...")) {
		if (ImGui::BeginMenu("Before selected", selectedIndex_ >= 0)) {
			changed |= DrawAddMenu(manager, static_cast<size_t>(selectedIndex_));
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("After selected", !changed && selectedIndex_ >= 0)) {
			changed |= DrawAddMenu(manager, static_cast<size_t>(selectedIndex_) + 1);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("At end", !changed)) {
			changed |= DrawAddMenu(manager, manager.GetSystemNames().size());
			ImGui::EndMenu();
		}
		ImGui::EndCombo();
	}
#else
	static_cast<void>(manager);
#endif // USE_IMGUI
	return changed;
}

bool SystemListPanel::DrawSystemList(ECS::SystemManager& manager) {
	bool changed = false;
#ifdef USE_IMGUI
	const std::vector<std::string> names = manager.GetSystemNames();
	if (ImGui::BeginChild("SystemRows", ImVec2(0, 0))) {
		if (names.empty()) ImGui::TextDisabled("No systems. Use Add system to add one.");
		for (size_t i = 0; i < names.size(); ++i) {
			ImGui::PushID(static_cast<int>(i));
			const std::string label = "::  " + std::to_string(i + 1) + ". " + names[i];
			if (ImGui::Selectable(label.c_str(), selectedIndex_ == static_cast<int>(i),
				ImGuiSelectableFlags_None, ImVec2(0, ImGui::GetFrameHeight()))) {
				selectedIndex_ = static_cast<int>(i);
			}
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) selectedIndex_ = static_cast<int>(i);
			if (scrollToSelection_ && selectedIndex_ == static_cast<int>(i)) {
				ImGui::SetScrollHereY();
				scrollToSelection_ = false;
			}

			if (ImGui::BeginDragDropSource()) {
				selectedIndex_ = static_cast<int>(i);
				const SystemDragPayload payload{ &manager, i };
				ImGui::SetDragDropPayload("SYSTEM_ROW", &payload, sizeof(payload));
				ImGui::Text("Move %s", names[i].c_str());
				ImGui::EndDragDropSource();
			}

			size_t from = 0, to = 0;
			if (AcceptReorderDrop(manager, i, names.size(), false, from, to)) {
				changed = manager.EditMoveSystem(from, to);
			}

			if (!changed && ImGui::BeginPopupContextItem("SystemContext")) {
				if (ImGui::BeginMenu("Add before")) {
					changed |= DrawAddMenu(manager, i);
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Add after", !changed)) {
					changed |= DrawAddMenu(manager, i + 1);
					ImGui::EndMenu();
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Remove", nullptr, false, !changed)) {
					changed = manager.EditRemoveSystem(i);
				}
				ImGui::EndPopup();
			}
			ImGui::PopID();
			// 構成変更後は古い一覧のインデックスを使わず、次のフレームで再描画する。
			if (changed) break;
		}
		if (!changed && !names.empty()) {
			// 最終行より下の余白にもドロップできるようにする。
			ImGui::InvisibleButton("AppendDrop", ImVec2(ImGui::GetContentRegionAvail().x,
				std::max(ImGui::GetFrameHeight(), ImGui::GetContentRegionAvail().y)));
			size_t from = 0, to = 0;
			if (AcceptReorderDrop(manager, names.size(), names.size(), true, from, to)) {
				changed = manager.EditMoveSystem(from, to);
			}
		}
	}
	ImGui::EndChild();
#else
	static_cast<void>(manager);
#endif // USE_IMGUI
	return changed;
}

}
}
