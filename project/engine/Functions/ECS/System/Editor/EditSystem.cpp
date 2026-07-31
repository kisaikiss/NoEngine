#include "EditSystem.h"
#include "engine/Editor/ComponentUI.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"
#include "engine/Editor/DataDriven/PrefabSerializer.h"
#include "engine/Editor/ReflectionMacros.h"
#include "engine/Editor/EditorCommandOperator.h"
#include "engine/Functions/Scene/SceneNameComponent.h"
#include "engine/Functions/Command/EditCommand/AddComponentCommand.h"
#include "engine/Functions/Command/EditCommand/InstantiateEntityCommand.h"
#include "engine/Functions/Command/EditCommand/DeleteEntityCommand.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "engine/Functions/ECS/Component/Common/Transform2DComponent.h"

#include "engine/Functions/Input/input.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

#include <map>
#include <algorithm>
#include <cctype>

namespace {
const std::string sDirectoryPath = "resources/game/Scenes/";
}

namespace NoEngine {
namespace ECS {

void EditSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	if (!FirstLoaded_) {
		LoadFile(registry);

		// Prefab一覧(DrawPrefabWindow / AssetBrowserWindow)からの「編集」要求を
		// EditSystemが受け取れるようにコールバックを登録する
		Editor::SetEditPrefabCallback([this](ECS::Registry& reg, const std::string& prefabPath) {
			BeginEditPrefab(reg, prefabPath);
			});

		FirstLoaded_ = true;
	}
#ifdef USE_IMGUI

	EnsureUniqueEditTagNames(registry);
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Save")) {
				SaveFile(registry, Editor::SaveScene(registry));
			}
			if (ImGui::MenuItem("Load")) {
				LoadFile(registry);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	DrawHierarchyWindow(registry);

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

	// --- Prefab編集ウィンドウ ---
	// ComponentUI.cppのDrawComponentUIをそのまま再利用し、
	// Prefab本体を一時Entityとしてロードして編集する。
	if (editingPrefabEntity_ != ECS::INVALID_ENTITY) {
		ImGui::Begin("Prefab Editor");
		ImGui::TextDisabled("%s", editingPrefabPath_.c_str());
		ImGui::Separator();

		DrawComponentUI(registry, editingPrefabEntity_);

		ImGui::Separator();
		if (ImGui::Button("Save & Apply")) {
			// 編集結果をPrefabファイルへ書き戻す
			nlohmann::json prefabJson = Editor::SaveEntityToJson(registry, editingPrefabEntity_);

			std::ofstream ofs(editingPrefabPath_);
			ofs << prefabJson.dump(4);
			ofs.close();

			// 既にシーンに配置済みの、同じPrefab由来のEntityへ変更を反映する
			Editor::ApplyPrefabToInstances(registry, editingPrefabPath_, prefabJson);

			Editor::LoadPrefabsFromDirectory();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			EndEditPrefab(registry);
		}

		ImGui::End();
	}

	// 選択しているEntityを取得
	for (auto e : registry.View<Editor::EditTag, Editor::EditSelectedTag>()) {
		editorState_.selectedEntity = e;
	}

	// 選択しているEntityに対して何らかの操作をInputによって行う //
	// 削除
	if (Input::Keyboard::IsTrigger(VK_DELETE)) {
		if (!ImGui::IsAnyItemActive() && editorState_.selectedEntity != ECS::INVALID_ENTITY) {
			Editor::EditorCommandOperator::AddCommand(std::make_unique<Command::DeleteEntityCommand>(registry, editorState_.selectedEntity));
			registry.DestroyEntity(editorState_.selectedEntity);
			LogInfo("DestroyEntity name : " + registry.GetComponent<Editor::EditTag>(editorState_.selectedEntity)->name);
			editorState_.selectedEntity = ECS::INVALID_ENTITY;
		}
	}

	// コピーやペーストを毎フレーム呼ばないためのインターバル計算
	if (timeInterval_ > 0.0f) {
		timeInterval_ -= deltaTime;
	}

	// コピー（選択エンティティ＋その子孫を全て再帰的にコピーする）
	if (Input::Keyboard::IsPress(VK_LCONTROL) && Input::Keyboard::IsPress('C')) {
		if (editorState_.selectedEntity != ECS::INVALID_ENTITY && editorState_.selectedEntity != editingPrefabEntity_) {
			if (!ImGui::IsAnyItemActive() && timeInterval_ <= 0.0f) {
				std::vector<Entity> subtree;
				CollectSubtreeEntities(registry, editorState_.selectedEntity, subtree);

				// Entity -> subtree内でのインデックス（貼り付け時に親子関係を復元するため）
				std::unordered_map<Entity, int> indexOf;
				indexOf.reserve(subtree.size() * 2);
				for (size_t i = 0; i < subtree.size(); ++i) {
					indexOf[subtree[i]] = static_cast<int>(i);
				}

				nlohmann::json nodesJson = nlohmann::json::array();
				for (auto e : subtree) {
					auto* tag = registry.GetComponent<Editor::EditTag>(e);
					nlohmann::json nodeJson;
					nodeJson["entity"] = Editor::SaveEntityToJson(registry, e);
					auto it = (tag && tag->parent != ECS::INVALID_ENTITY) ? indexOf.find(tag->parent) : indexOf.end();
					nodeJson["parentIndex"] = (it != indexOf.end()) ? it->second : -1;
					nodesJson.push_back(nodeJson);
				}

				copyObject_ = nlohmann::json{ {"nodes", nodesJson} };
				LogInfo("CopyObject name : " + registry.GetComponent<Editor::EditTag>(editorState_.selectedEntity)->name
					+ " (" + std::to_string(subtree.size()) + " entities)");
				static constexpr float kIntervalTime = 0.3f;
				timeInterval_ = kIntervalTime;
			}
		}
	}

	// ペースト（コピーした親子関係をそのまま新しいEntityとして復元する）
	if (Input::Keyboard::IsPress(VK_LCONTROL) && Input::Keyboard::IsPress('V')) {
		if (timeInterval_ <= 0.0f) {
			if (!ImGui::IsAnyItemActive() && !copyObject_.empty() && copyObject_.contains("nodes")) {
				const auto& nodes = copyObject_["nodes"];

				std::vector<Entity> newEntities;
				newEntities.reserve(nodes.size());

				// 1) まず全Entityを生成してデータを流し込む
				for (const auto& node : nodes) {
					Entity newE = registry.GenerateEntity();
					Editor::LoadEntityFromJson(registry, newE, node["entity"]);
					newEntities.push_back(newE);
				}

				// 2) 新しいEntity IDで親子関係を張り直す
				for (size_t i = 0; i < newEntities.size(); ++i) {
					int parentIndex = nodes[i].value("parentIndex", -1);
					Entity newParent = (parentIndex >= 0) ? newEntities[static_cast<size_t>(parentIndex)] : ECS::INVALID_ENTITY;
					SetEntityParent(registry, newEntities[i], newParent);
					Editor::EditorCommandOperator::AddCommand(std::make_unique<Command::InstantiateEntityCommand>(registry, newEntities[i]));
				}

				// 3) コピー元のルートに対応する新規Entityだけを選択状態にする
				if (!newEntities.empty()) {
					registry.AddComponent<Editor::EditSelectedTag>(newEntities.front());
					editorState_.selectedEntity = newEntities.front();
				}

				LogInfo("PasteObject : " + std::to_string(newEntities.size()) + " entities");
				static constexpr float kIntervalTime = 1.f;
				timeInterval_ = kIntervalTime;
			}
		}
	}

#else
	static_cast<void>(registry);
#endif // USE_IMGUI
}

void EditSystem::BeginEditPrefab(Registry& registry, const std::string& prefabPath) {
	// 既に別のPrefabを編集中だった場合は、その一時Entityを破棄してから切り替える
	if (editingPrefabEntity_ != ECS::INVALID_ENTITY) {
		registry.DestroyEntity(editingPrefabEntity_);
		editingPrefabEntity_ = ECS::INVALID_ENTITY;
	}

	editingPrefabEntity_ = Editor::LoadPrefabForEditing(registry, prefabPath);
	editingPrefabPath_ = prefabPath;
}

void EditSystem::EndEditPrefab(Registry& registry) {
	if (editingPrefabEntity_ != ECS::INVALID_ENTITY) {
		registry.DestroyEntity(editingPrefabEntity_);
		editingPrefabEntity_ = ECS::INVALID_ENTITY;
	}
	editingPrefabPath_.clear();
}

void EditSystem::SaveFile(Registry& registry, nlohmann::json j) {
	std::filesystem::path dir(sDirectoryPath);
	if (!std::filesystem::exists(dir)) {
		std::filesystem::create_directory(dir);
	}

	std::string sceneName;
	auto view = registry.View<SceneNameComponent>();
	for (auto entity : view) {
		auto* nameComp = registry.GetComponent<SceneNameComponent>(entity);
		sceneName = nameComp->GetName();
	}
	std::string filePath = sDirectoryPath + sceneName + ".json";
	std::ofstream file(filePath);
	if (!file.is_open()) {
		LogError("Failed to open file for writing");
		return;
	}

	file << j.dump(4);
}

void EditSystem::LoadFile(Registry& registry) {
	std::string sceneName;
	auto view = registry.View<SceneNameComponent>();
	for (auto entity : view) {
		auto* nameComp = registry.GetComponent<SceneNameComponent>(entity);
		sceneName = nameComp->GetName();
	}
	std::string filePath = sDirectoryPath + sceneName + ".json";

	std::ifstream file(filePath);
	if (!file.is_open()) {
		LogError("Failed to open scene file:" + filePath);
		return;
	}

	nlohmann::json scene;
	file >> scene;

	Editor::LoadScene(registry, scene);
}

void EditSystem::DrawHierarchyWindow(Registry& registry) {
#ifdef USE_IMGUI
	ImGui::Begin("Hierarchy");

	if (ImGui::BeginPopupContextWindow()) {
		if (ImGui::MenuItem("Generate newEntity")) {
			Entity e = registry.GenerateEntity();
			auto* tag = registry.AddComponent<Editor::EditTag>(e);
			tag->name = "newEntity";
			registry.AddComponent<Editor::EditSelectedTag>(e);
			Editor::EditorCommandOperator::AddCommand(std::make_unique<Command::InstantiateEntityCommand>(registry, e));
		}
		ImGui::EndPopup();
	}

	// 親Entity -> 子Entity一覧 のマップをEditTag::parentから毎フレーム構築する
	std::unordered_map<Entity, std::vector<Entity>> childrenMap;
	std::vector<Entity> roots;

	auto view = registry.View<Editor::EditTag>();
	for (auto e : view) {
		auto* tag = registry.GetComponent<Editor::EditTag>(e);
		if (tag->parent == ECS::INVALID_ENTITY) {
			roots.push_back(e);
		} else {
			childrenMap[tag->parent].push_back(e);
		}
	}

	// Shift+Click範囲選択用に、現在のツリー表示順（プリオーダー）を作っておく
	hierarchyOrder_.clear();
	for (auto e : roots) {
		BuildHierarchyOrder(e, childrenMap, hierarchyOrder_);
	}

	for (auto e : roots) {
		DrawEntityNode(registry, e, childrenMap);
	}

	// ウィンドウの何もない場所をクリックしたら選択解除する
	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered()) {
		editorState_.hierarchySelection.clear();
		editorState_.lastClickedEntity = ECS::INVALID_ENTITY;
	}

	// ウィンドウ下部の余白：ここへドロップすると親を外してルートに戻せる
	ImGui::Dummy(ImVec2(-1.0f, ImGui::GetContentRegionAvail().y > 0.0f ? ImGui::GetContentRegionAvail().y : 20.0f));
	if (ImGui::BeginDragDropTarget()) {
		HandleHierarchyDrop(registry, ECS::INVALID_ENTITY);
		ImGui::EndDragDropTarget();
	}

	ImGui::End();
#else
	static_cast<void>(registry);
#endif
}

void EditSystem::DrawEntityNode(ECS::Registry& registry, ECS::Entity e, const std::unordered_map<ECS::Entity, std::vector<ECS::Entity>>& childrenMap) {
#ifdef USE_IMGUI
	auto* tag = registry.GetComponent<Editor::EditTag>(e);
	if (!tag) return;

	auto childrenIt = childrenMap.find(e);
	bool hasChildren = (childrenIt != childrenMap.end());
	bool selected = editorState_.hierarchySelection.count(e) > 0;

	// 子を持つEntityはツリーとして開閉できるようにし、持たないEntityは葉として表示する
	// （＝親は子を入れたフォルダーのように振る舞う）
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
	if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	if (selected) flags |= ImGuiTreeNodeFlags_Selected;

	std::string label = tag->name;
	bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(e)), flags, "%s", label.c_str());

	// クリックで選択（矢印の開閉クリックとは区別する。単一/Ctrl+追加解除/Shift+範囲選択をまとめて処理する）
	// 注意：IsItemClickedはマウス押下のタイミングで発火するため、ここで即座に単一選択へ
	// 潰してしまうと、その直後のBeginDragDropSourceが複数選択を拾えなくなる（＝1個しかドラッグできない）。
	// そのため「既に複数選択済みの1つを無修飾でクリックした」場合だけ選択確定を保留し、
	// 実際にドラッグへ移行しなかった（マウスを離した）時点で単一選択に還元する。
	bool alreadySelected = editorState_.hierarchySelection.count(e) > 0;
	bool multiSelectActive = editorState_.hierarchySelection.size() > 1;
	bool ctrlHeld = ImGui::GetIO().KeyCtrl;
	bool shiftHeld = ImGui::GetIO().KeyShift;

	if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen()) {
		if (alreadySelected && multiSelectActive && !ctrlHeld && !shiftHeld) {
			pendingSingleSelectEntity_ = e;
		} else {
			HandleHierarchyClick(registry, e);
			pendingSingleSelectEntity_ = ECS::INVALID_ENTITY;
		}
	}

	// ドラッグ開始（複数選択中にその一員をドラッグした場合は選択されている全Entityを一緒に運ぶ）
	if (ImGui::BeginDragDropSource()) {
		// ドラッグが実際に成立したので、単一選択への還元は行わない
		pendingSingleSelectEntity_ = ECS::INVALID_ENTITY;

		std::vector<Entity> dragEntities;
		if (editorState_.hierarchySelection.count(e) && editorState_.hierarchySelection.size() > 1) {
			dragEntities.assign(editorState_.hierarchySelection.begin(), editorState_.hierarchySelection.end());
		} else {
			dragEntities.push_back(e);
		}

		ImGui::SetDragDropPayload("ENTITY_MULTI", dragEntities.data(), dragEntities.size() * sizeof(Entity));
		if (dragEntities.size() > 1) {
			ImGui::Text("%zu entities", dragEntities.size());
		} else {
			ImGui::Text("%s", tag->name.c_str());
		}
		ImGui::EndDragDropSource();
	}

	// ドラッグに発展せずマウスを離した場合は、ここで単一選択に還元する
	if (pendingSingleSelectEntity_ == e && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
		HandleHierarchyClick(registry, e);
		pendingSingleSelectEntity_ = ECS::INVALID_ENTITY;
	}

	// ドロップ受け取り（このEntityを親として付け替える。複数選択をまとめて運んできた場合も対応）
	if (ImGui::BeginDragDropTarget()) {
		HandleHierarchyDrop(registry, e);
		ImGui::EndDragDropTarget();
	}

	// 右クリックメニュー
	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::MenuItem("Generate newEntity")) {
			Entity newE = registry.GenerateEntity();
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
			// 削除処理
			Editor::EditorCommandOperator::AddCommand(std::make_unique<Command::DeleteEntityCommand>(registry, e));
			std::vector<Entity> children;
			CollectSubtreeEntities(registry, e, children);
			for (auto child : children) {
				registry.DestroyEntity(child);
			}
			registry.DestroyEntity(e);
			editorState_.hierarchySelection.erase(e);
			if (editorState_.selectedEntity == e) editorState_.selectedEntity = ECS::INVALID_ENTITY;
			if (editorState_.lastClickedEntity == e) editorState_.lastClickedEntity = ECS::INVALID_ENTITY;
			if (pendingSingleSelectEntity_ == e) pendingSingleSelectEntity_ = ECS::INVALID_ENTITY;
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

void EditSystem::HandleHierarchyClick(ECS::Registry& registry, ECS::Entity e) {
#ifdef USE_IMGUI
	const bool ctrl = ImGui::GetIO().KeyCtrl;
	const bool shift = ImGui::GetIO().KeyShift;

	auto setPrimary = [&](ECS::Entity primary) {
		// InspectorWindow用の代表選択はEditSelectedTagと1対1（既存の排他処理に乗せる）
		editorState_.selectedEntity = primary;
		if (primary != ECS::INVALID_ENTITY) {
			registry.AddComponent<Editor::EditSelectedTag>(primary);
		}
		};

	if (shift && editorState_.lastClickedEntity != ECS::INVALID_ENTITY) {
		// 範囲選択：直前にクリックしたEntity～今回クリックしたEntityまでを表示順で選択する
		auto itA = std::find(hierarchyOrder_.begin(), hierarchyOrder_.end(), editorState_.lastClickedEntity);
		auto itB = std::find(hierarchyOrder_.begin(), hierarchyOrder_.end(), e);
		if (itA != hierarchyOrder_.end() && itB != hierarchyOrder_.end()) {
			size_t a = static_cast<size_t>(std::distance(hierarchyOrder_.begin(), itA));
			size_t b = static_cast<size_t>(std::distance(hierarchyOrder_.begin(), itB));
			if (a > b) std::swap(a, b);

			editorState_.hierarchySelection.clear();
			for (size_t i = a; i <= b; ++i) {
				editorState_.hierarchySelection.insert(hierarchyOrder_[i]);
			}
			setPrimary(e);
		} else {
			editorState_.hierarchySelection = { e };
			setPrimary(e);
		}
		// lastClickedEntity_ はShiftの基準として更新しない（Unity挙動に合わせて起点を維持する）
		return;
	}

	if (ctrl) {
		// Ctrl+Click：選択の追加/解除トグル
		if (editorState_.hierarchySelection.count(e)) {
			editorState_.hierarchySelection.erase(e);
			if (editorState_.selectedEntity == e) {
				registry.RemoveComponent<Editor::EditSelectedTag>(e);
				Entity newPrimary = editorState_.hierarchySelection.empty()
					? ECS::INVALID_ENTITY
					: *editorState_.hierarchySelection.rbegin();
				setPrimary(newPrimary);
			}
		} else {
			editorState_.hierarchySelection.insert(e);
			setPrimary(e);
		}
		editorState_.lastClickedEntity = e;
		return;
	}

	// 通常クリック：単一選択
	editorState_.hierarchySelection = { e };
	setPrimary(e);
	editorState_.lastClickedEntity = e;
#else
	static_cast<void>(registry);
	static_cast<void>(e);
#endif // USE_IMGUI
}

void EditSystem::BuildHierarchyOrder(ECS::Entity e, const std::unordered_map<ECS::Entity, std::vector<ECS::Entity>>& childrenMap, std::vector<ECS::Entity>& outOrder) {
	outOrder.push_back(e);
	auto it = childrenMap.find(e);
	if (it == childrenMap.end()) return;
	for (auto child : it->second) {
		BuildHierarchyOrder(child, childrenMap, outOrder);
	}
}

void EditSystem::HandleHierarchyDrop(ECS::Registry& registry, ECS::Entity newParent) {
#ifdef USE_IMGUI
	const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_MULTI");
	if (!payload) return;

	const Entity* dropped = reinterpret_cast<const Entity*>(payload->Data);
	const size_t count = payload->DataSize / sizeof(Entity);

	for (size_t i = 0; i < count; ++i) {
		Entity candidate = dropped[i];
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


void EditSystem::DrawAddComponentMenu(Registry& registry, Entity entity) {
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

void EditSystem::EnsureUniqueEditTagNames(Registry& registry) {
	// 1) 収集フェーズ
	std::vector<std::pair<Entity, std::string>> items;
	items.reserve(1024); // 予想件数に合わせて調整

	auto view = registry.View<Editor::EditTag>();
	for (auto e : view) {
		auto* tag = registry.GetComponent<Editor::EditTag>(e);
		if (!tag) continue;
		items.emplace_back(e, tag->name);
	}

	// 2) 出現回数と次のインデックスを準備
	std::unordered_map<std::string, int> count;
	count.reserve(items.size() * 2);

	// まずは原名のカウント（同名が何個あるか）
	for (auto& p : items) {
		count[p.second] += 1;
	}

	// 3) 一意化フェーズ
	// baseName -> nextIndex を保持しておく（既に base, base_1, base_2 がある場合に備える）
	std::unordered_map<std::string, int> nextIndex;
	nextIndex.reserve(count.size() * 2);

	// 初期化: 既に1個しかない名前は nextIndex=1、複数ある名前は nextIndex=1
	for (auto& kv : count) {
		nextIndex[kv.first] = 1;
	}

	// 実際に名前を割り当てる（元の順序を保つ）
	for (auto& p : items) {
		Entity e = p.first;
		std::string& orig = p.second;

		// その名前がユニークならそのまま（かつ次Indexは1のまま）
		if (count[orig] == 1) {
			// 何もしない
			continue;
		}

		// 複数ある場合、最初に出現したものは元名を使い、以降に suffix を付ける方針にする
		int idx = nextIndex[orig];
		if (idx == 1) {
			// 最初の出現は元名を使う（ただし既に他で使われている可能性がある場合はチェック）
			// マップで既に使われているか確認するために usedNames を使う方法もあるが、
			// ここでは単純に nextIndex をインクリメントして次から suffix を付ける。
			nextIndex[orig] = 2;
			// mark as "used" by setting count to 0 so we don't treat it again (optional)
			// ただし今回は count は総数なのでそのままにする
			continue;
		}

		// suffix を付ける
		std::string newName;
		// 予め reserve しておく（元名長 + '_' + digits）
		newName.reserve(orig.size() + 8);
		while (true) {
			newName = orig;
			newName.push_back('_');
			newName += std::to_string(idx);

			// 確認: 既に同じ newName が存在していないかを nextIndex map で確認
			// nextIndex に newName が無ければ未使用とみなす
			if (nextIndex.find(newName) == nextIndex.end()) {
				// 決定
				nextIndex[newName] = 1; // newName を使ったので登録
				nextIndex[orig] = idx + 1; // 次に orig を使うときは idx+1
				break;
			}
			++idx;
		}

		// 実際にコンポーネントを書き換える（1回だけ GetComponent 呼ぶ）
		auto* tag = registry.GetComponent<Editor::EditTag>(e);
		if (tag) {
			tag->name = std::move(newName);
		}
	}
}

bool EditSystem::IsDescendantOf(ECS::Registry& registry, ECS::Entity ancestorCandidate, ECS::Entity entity) {
	// ancestorCandidate から親を辿って entity に到達できれば、
	// entity は ancestorCandidate の祖先（＝ancestorCandidateはentityの子孫）ということになる
	ECS::Entity current = ancestorCandidate;
	while (current != ECS::INVALID_ENTITY) {
		if (current == entity) return true;
		auto* tag = registry.GetComponent<Editor::EditTag>(current);
		current = tag ? tag->parent : ECS::INVALID_ENTITY;
	}
	return false;
}

void EditSystem::SetEntityParent(ECS::Registry& registry, ECS::Entity child, ECS::Entity newParent) {
	if (child == ECS::INVALID_ENTITY || child == newParent) return;

	// 自分の子孫を自分の親にしようとした場合は循環参照になるので中止する
	if (newParent != ECS::INVALID_ENTITY && IsDescendantOf(registry, newParent, child)) {
		LogWarning("EditSystem: cannot parent an entity to its own descendant.");
		return;
	}

	auto* tag = registry.GetComponent<Editor::EditTag>(child);
	if (!tag) return;
	tag->parent = newParent;

	// 実際のTransform階層（ワールド座標計算に使われる parent）も、
	// 型が一致する場合のみ同期する。フォルダ（Transformを持たないEntity）へ
	// 入れた場合は見た目上の階層分けのみに留まり、座標計算には影響しない。
	if (auto* childTransform = registry.GetComponent<Component::TransformComponent>(child)) {
		Component::TransformComponent* parentTransform =
			(newParent != ECS::INVALID_ENTITY) ? registry.GetComponent<Component::TransformComponent>(newParent) : nullptr;
		childTransform->parent = (newParent == ECS::INVALID_ENTITY || parentTransform) ? newParent : ECS::INVALID_ENTITY;
	} else if (auto* childTransform2D = registry.GetComponent<Component::Transform2DComponent>(child)) {
		Component::Transform2DComponent* parentTransform2D =
			(newParent != ECS::INVALID_ENTITY) ? registry.GetComponent<Component::Transform2DComponent>(newParent) : nullptr;
		childTransform2D->parent = (newParent == ECS::INVALID_ENTITY || parentTransform2D) ? newParent : ECS::INVALID_ENTITY;
	}
}

void EditSystem::CollectSubtreeEntities(ECS::Registry& registry, ECS::Entity root, std::vector<ECS::Entity>& outEntities) {
	outEntities.push_back(root);

	// EditTagを持つ全Entityをスキャンしてrootのparentになっているものを再帰的に集める
	// （エディタでの操作前提のためO(N)スキャンで問題ない規模を想定）
	auto view = registry.View<Editor::EditTag>();
	for (auto e : view) {
		auto* tag = registry.GetComponent<Editor::EditTag>(e);
		if (tag && tag->parent == root) {
			CollectSubtreeEntities(registry, e, outEntities);
		}
	}
}
}
}