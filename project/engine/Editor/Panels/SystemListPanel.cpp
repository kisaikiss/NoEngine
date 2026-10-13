#include "stdafx.h"
#include "SystemListPanel.h"
#include "engine/Functions/ECS/System/SystemManager.h"
#include "engine/Functions/ECS/System/SystemCatalog.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

namespace NoEngine {
namespace Editor {

#ifdef USE_IMGUI
namespace {

// 直前に描画した項目(index番目)へのドロップを受け付け、並べ替えの from/to を求める。
// 項目の上半分にドロップなら手前、下半分なら後ろへ挿入する。
bool AcceptReorderDrop(size_t targetIndex, size_t count, size_t& outFrom, size_t& outTo) {
	bool accepted = false;
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SYSTEM_INDEX")) {
			const size_t from = *static_cast<const size_t*>(payload->Data);
			if (payload->IsDelivery() && from < count && from != targetIndex) {
				const float midpoint = (ImGui::GetItemRectMin().y + ImGui::GetItemRectMax().y) * 0.5f;
				const bool dropAfter = ImGui::GetMousePos().y >= midpoint;

				size_t to = targetIndex;
				if (dropAfter && from > targetIndex) ++to;
				else if (!dropAfter && from < targetIndex) --to;

				outFrom = from;
				outTo = to;
				accepted = true;
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
	ImGui::Begin("Systems");
	changed |= DrawAddCombo(manager);
	changed |= DrawSystemList(manager);
	ImGui::End();
#else
	static_cast<void>(manager);
#endif // USE_IMGUI
	return changed;
}

bool SystemListPanel::DrawAddCombo(ECS::SystemManager& manager) {
	bool changed = false;
#ifdef USE_IMGUI
	if (ImGui::BeginCombo("Add system", "Select system...")) {
		for (const auto& [category, names] : ECS::SystemCatalog::Get().GroupedByCategory()) {
			ImGui::TextDisabled("%s", category.c_str());
			for (const auto& name : names) {
				const bool alreadyAdded = manager.HasSystem(name);
				ImGui::Indent();
				if (ImGui::Selectable(name.c_str(), false,
					alreadyAdded ? ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None) && !alreadyAdded) {
					if (manager.AddSystemByName(name)) changed = true;
				}
				ImGui::Unindent();
			}
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

	for (size_t i = 0; i < names.size(); ++i) {
		ImGui::PushID(static_cast<int>(i));

		ImGui::Text("%zu. %s", i + 1, names[i].c_str());

		// ドラッグ元
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
			ImGui::SetDragDropPayload("SYSTEM_INDEX", &i, sizeof(i));
			ImGui::Text("Move %s", names[i].c_str());
			ImGui::EndDragDropSource();
		}

		// ドロップ先(並べ替え)
		size_t from = 0, to = 0;
		if (AcceptReorderDrop(i, names.size(), from, to)) {
			manager.MoveSystem(from, to);
			changed = true;
			ImGui::PopID();
			break; // 並びが変わったのでこのフレームの描画は打ち切る
		}

		// 右クリックメニュー(削除)
		if (ImGui::BeginPopupContextItem("SystemContext")) {
			if (ImGui::MenuItem("Remove")) {
				manager.RemoveSystem(i);
				changed = true;
				ImGui::EndPopup();
				ImGui::PopID();
				break;
			}
			ImGui::EndPopup();
		}

		ImGui::PopID();
	}
#else
	static_cast<void>(manager);
#endif // USE_IMGUI
	return changed;
}

}
}