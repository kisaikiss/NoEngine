#include "HierarchyPanel.h"
#include "EntityHierarchyUtils.h"
#include "engine/Editor/ReflectionMacros.h"
#include "engine/Editor/EditorCommandOperator.h"
#include "engine/Functions/Command/EditCommand/InstantiateEntityCommand.h"

#include <algorithm>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

namespace NoEngine {
namespace Editor {

void HierarchyPanel::Draw(ECS::Registry& registry) {
#ifdef USE_IMGUI
	ImGui::Begin("Hierarchy");

	if (ImGui::BeginPopupContextWindow()) {
		if (ImGui::MenuItem("Generate newEntity")) {
			ECS::Entity e = registry.GenerateEntity();
			auto* tag = registry.AddComponent<Editor::EditTag>(e);
			tag->name = "newEntity";
			registry.AddComponent<Editor::EditSelectedTag>(e);
			Editor::EditorCommandOperator::AddCommand(std::make_unique<Command::InstantiateEntityCommand>(registry, e));
		}
		ImGui::EndPopup();
	}

	// 親Entity -> 子Entity一覧 のマップをEditTag::parentから毎フレーム構築する
	std::unordered_map<ECS::Entity, std::vector<ECS::Entity>> childrenMap;
	std::vector<ECS::Entity> roots;
	BuildHierarchyMap(registry, childrenMap, roots);

	// Shift+Click範囲選択用に、現在のツリー表示順（プリオーダー）を作っておく
	hierarchyOrder_.clear();
	for (auto e : roots) {
		BuildPreOrder(e, childrenMap, hierarchyOrder_);
	}

	for (auto e : roots) {
		DrawEntityNode(registry, e, childrenMap);
	}

	// ウィンドウの何もない場所をクリックしたら選択解除する
	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered()) {
		selection_.clear();
		lastClickedEntity_ = ECS::INVALID_ENTITY;
	}

	// ウィンドウ下部の余白：ここへドロップすると親を外してルートに戻せる
	ImGui::Dummy(ImVec2(-1.0f, ImGui::GetContentRegionAvail().y > 0.0f ? ImGui::GetContentRegionAvail().y : 20.0f));
	if (ImGui::BeginDragDropTarget()) {
		HandleDrop(registry, ECS::INVALID_ENTITY);
		ImGui::EndDragDropTarget();
	}

	ImGui::End();
#else
	static_cast<void>(registry);
#endif // USE_IMGUI
}

void HierarchyPanel::ClearSelectionOf(ECS::Entity e) {
	selection_.erase(e);
	if (lastClickedEntity_ == e) lastClickedEntity_ = ECS::INVALID_ENTITY;
	if (pendingSingleSelectEntity_ == e) pendingSingleSelectEntity_ = ECS::INVALID_ENTITY;
}

void HierarchyPanel::DrawEntityNode(ECS::Registry& registry, ECS::Entity e, const std::unordered_map<ECS::Entity, std::vector<ECS::Entity>>& childrenMap) {
#ifdef USE_IMGUI
	auto* tag = registry.GetComponent<Editor::EditTag>(e);
	if (!tag) return;

	auto childrenIt = childrenMap.find(e);
	bool hasChildren = (childrenIt != childrenMap.end());
	bool selected = selection_.count(e) > 0;

	// 子を持つEntityはツリーとして開閉できるようにし、持たないEntityは葉として表示する
	// （＝親は子を入れたフォルダーのように振る舞う）
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
	if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	if (selected) flags |= ImGuiTreeNodeFlags_Selected;

	bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(e)), flags, "%s", tag->name.c_str());

	// クリックで選択（矢印の開閉クリックとは区別する。単一/Ctrl+追加解除/Shift+範囲選択をまとめて処理する）
	// 注意：IsItemClickedはマウス押下のタイミングで発火するため、ここで即座に単一選択へ
	// 潰してしまうと、その直後のBeginDragDropSourceが複数選択を拾えなくなる（＝1個しかドラッグできない）。
	// そのため「既に複数選択済みの1つを無修飾でクリックした」場合だけ選択確定を保留し、
	// 実際にドラッグへ移行しなかった（マウスを離した）時点で単一選択に還元する。
	bool alreadySelected = selection_.count(e) > 0;
	bool multiSelectActive = selection_.size() > 1;
	bool ctrlHeld = ImGui::GetIO().KeyCtrl;
	bool shiftHeld = ImGui::GetIO().KeyShift;

	if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen()) {
		if (alreadySelected && multiSelectActive && !ctrlHeld && !shiftHeld) {
			pendingSingleSelectEntity_ = e;
		} else {
			HandleClick(registry, e);
			pendingSingleSelectEntity_ = ECS::INVALID_ENTITY;
		}
	}

	// ドラッグ開始（複数選択中にその一員をドラッグした場合は選択されている全Entityを一緒に運ぶ）
	if (ImGui::BeginDragDropSource()) {
		// ドラッグが実際に成立したので、単一選択への還元は行わない
		pendingSingleSelectEntity_ = ECS::INVALID_ENTITY;

		std::vector<ECS::Entity> dragEntities;
		if (selection_.count(e) && selection_.size() > 1) {
			dragEntities.assign(selection_.begin(), selection_.end());
		} else {
			dragEntities.push_back(e);
		}

		ImGui::SetDragDropPayload("ENTITY_MULTI", dragEntities.data(), dragEntities.size() * sizeof(ECS::Entity));
		if (dragEntities.size() > 1) {
			ImGui::Text("%zu entities", dragEntities.size());
		} else {
			ImGui::Text("%s", tag->name.c_str());
		}
		ImGui::EndDragDropSource();
	}

	// ドラッグに発展せずマウスを離した場合は、ここで単一選択に還元する
	if (pendingSingleSelectEntity_ == e && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
		HandleClick(registry, e);
		pendingSingleSelectEntity_ = ECS::INVALID_ENTITY;
	}

	// ドロップ受け取り（このEntityを親として付け替える。複数選択をまとめて運んできた場合も対応）
	if (ImGui::BeginDragDropTarget()) {
		HandleDrop(registry, e);
		ImGui::EndDragDropTarget();
	}

	// 右クリックメニュー
	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::MenuItem("Generate newEntity")) {
			ECS::Entity newE = registry.GenerateEntity();
			auto* newTag = registry.AddComponent<Editor::EditTag>(newE);
			newTag->name = "newEntity";
			newTag->parent = e;
			registry.AddComponent<Editor::EditSelectedTag>(newE);
			Editor::EditorCommandOperator::AddCommand(std::make_unique<Command::InstantiateEntityCommand>(registry, newE));
		}
		if (ImGui::MenuItem("Rename")) {
			ImGui::OpenPopup("RenameEntityPopup");
		}
		if (ImGui::MenuItem("Delete")) {
			// 削除処理（子孫ごと消す。Undo/Redoが子孫にも効くよう、破棄するEntity毎にコマンドを記録する）
			std::vector<ECS::Entity> subtree;
			CollectSubtreeEntities(registry, e, subtree);
			for (auto member : subtree) {
				ClearSelectionOf(member);
			}
			DeleteEntitiesWithUndo(registry, subtree);
			ImGui::EndPopup();
			if (open && hasChildren) ImGui::TreePop();
			return; // 既に削除したので以降の UI は無効
		}
		ImGui::EndPopup();
	}

	// Rename ポップアップ（簡易）
	if (ImGui::BeginPopupModal("RenameEntityPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		static char buf[256] = "";
		strncpy_s(buf, tag->name.c_str(), sizeof(buf));
		if (ImGui::InputText("Name", buf, sizeof(buf))) {}
		if (ImGui::Button("OK")) {
			tag->name = std::string(buf);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (open && hasChildren) {
		for (auto child : childrenIt->second) {
			DrawEntityNode(registry, child, childrenMap);
		}
		ImGui::TreePop();
	}
#else
	static_cast<void>(registry);
	static_cast<void>(e);
	static_cast<void>(childrenMap);
#endif // USE_IMGUI
}

void HierarchyPanel::HandleClick(ECS::Registry& registry, ECS::Entity e) {
#ifdef USE_IMGUI
	const bool ctrl = ImGui::GetIO().KeyCtrl;
	const bool shift = ImGui::GetIO().KeyShift;

	auto setPrimary = [&](ECS::Entity primary) {
		// Inspector用の代表選択はEditSelectedTagと1対1（既存の排他処理により他Entityからは自動的に外れる）
		if (primary != ECS::INVALID_ENTITY) {
			registry.AddComponent<Editor::EditSelectedTag>(primary);
		}
		};

	if (shift && lastClickedEntity_ != ECS::INVALID_ENTITY) {
		// 範囲選択：直前にクリックしたEntity～今回クリックしたEntityまでを表示順で選択する
		auto itA = std::find(hierarchyOrder_.begin(), hierarchyOrder_.end(), lastClickedEntity_);
		auto itB = std::find(hierarchyOrder_.begin(), hierarchyOrder_.end(), e);
		if (itA != hierarchyOrder_.end() && itB != hierarchyOrder_.end()) {
			size_t a = static_cast<size_t>(std::distance(hierarchyOrder_.begin(), itA));
			size_t b = static_cast<size_t>(std::distance(hierarchyOrder_.begin(), itB));
			if (a > b) std::swap(a, b);

			selection_.clear();
			for (size_t i = a; i <= b; ++i) {
				selection_.insert(hierarchyOrder_[i]);
			}
			setPrimary(e);
		} else {
			selection_ = { e };
			setPrimary(e);
		}
		// lastClickedEntity_ はShiftの基準として更新しない
		return;
	}

	if (ctrl) {
		// Ctrl+Click：選択の追加/解除トグル
		if (selection_.count(e)) {
			selection_.erase(e);
			if (registry.Has<Editor::EditSelectedTag>(e)) {
				registry.RemoveComponent<Editor::EditSelectedTag>(e);
				ECS::Entity newPrimary = selection_.empty() ? ECS::INVALID_ENTITY : *selection_.rbegin();
				setPrimary(newPrimary);
			}
		} else {
			selection_.insert(e);
			setPrimary(e);
		}
		lastClickedEntity_ = e;
		return;
	}

	// 通常クリック：単一選択
	selection_ = { e };
	setPrimary(e);
	lastClickedEntity_ = e;
#else
	static_cast<void>(registry);
	static_cast<void>(e);
#endif // USE_IMGUI
}

void HierarchyPanel::HandleDrop(ECS::Registry& registry, ECS::Entity newParent) {
#ifdef USE_IMGUI
	const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_MULTI");
	if (!payload) return;

	const ECS::Entity* dropped = reinterpret_cast<const ECS::Entity*>(payload->Data);
	const size_t count = payload->DataSize / sizeof(ECS::Entity);

	for (size_t i = 0; i < count; ++i) {
		ECS::Entity candidate = dropped[i];
		if (candidate == newParent) continue;

		// 選択の中で「他の選択Entityの子孫」になっているものはスキップする
		// （親側を付け替えれば子はそのまま一緒についてくるため、二重に付け替えない）
		bool coveredByAnotherSelected = false;
		for (size_t j = 0; j < count; ++j) {
			if (i == j) continue;
			if (IsDescendantOf(registry, candidate, dropped[j])) {
				coveredByAnotherSelected = true;
				break;
			}
		}
		if (coveredByAnotherSelected) continue;

		SetEntityParent(registry, candidate, newParent);
	}
#else
	static_cast<void>(registry);
	static_cast<void>(newParent);
#endif // USE_IMGUI
}

}
}