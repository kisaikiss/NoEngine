#include "EditSystem.h"
#include "engine/Editor/EditUtils.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"
#include "engine/Editor/DataDriven/PrefabSerializer.h"
#include "engine/Editor/ReflectionMacros.h"
#include "engine/Editor/EditorCommandOperator.h"
#include "engine/Functions/Scene/SceneNameComponent.h"
#include "engine/Functions/Command/EditCommand/AddComponentCommand.h"
#include "engine/Functions/Command/EditCommand/InstantiateEntityCommand.h"
#include "engine/Functions/Command/EditCommand/DeleteEntityCommand.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

namespace {
const std::string sDirectoryPath = "resources/game/Scenes/";
}

REFLECT_STRUCT_BEGIN(NoEngine::FolderTag)
REFLECT_STRUCT_END(NoEngine::FolderTag)
namespace NoEngine {
namespace ECS {

namespace {
struct EditorState {
	Entity selectedEntity = INVALID_ENTITY;
};

static EditorState sEditorState;

}

void EditSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	if (!FirstLoaded_) {
		LoadFile(registry);
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


#else
	static_cast<void>(registry);
#endif // USE_IMGUI
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
		if (ImGui::MenuItem("Create Folder (Root)")) {
			CreateFolder(registry, "NewFolder", "");
		}
		if (ImGui::MenuItem("Generate newEntity")) {
			Entity e = registry.GenerateEntity();
			auto* tag = registry.AddComponent<Editor::EditTag>(e);
			tag->name = "newEntity";
			registry.AddComponent<Editor::EditSelectedTag>(e);
			Editor::EditorCommandOperator::AddCommand(std::make_unique<Command::InstantiateEntityCommand>(registry, e));
		}
		ImGui::EndPopup();
	}

	FolderNode root;
	root.name = "";
	auto view = registry.View<Editor::EditTag>();

	for (auto e : view) {
		auto* tag = registry.GetComponent<Editor::EditTag>(e);

		// パスが空なら名前をパスとして扱う
		if (tag->path.empty()) tag->path = tag->name;

		AddEntityToFolder(root, registry, e);
	}

	DrawFolderNode(root, registry, "");

	ImGui::End();
#else
	static_cast<void>(registry);
#endif
}

void EditSystem::DrawFolderNode(FolderNode& node, ECS::Registry& registry, const std::string& currentPath) {
#ifdef USE_IMGUI

	const char* label = node.name.empty() ? "Scene" : node.name.c_str();

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
	if (node.name.empty()) flags |= ImGuiTreeNodeFlags_DefaultOpen;

	// フォルダ自体の Entity が選択されているか
	if (node.folderEntity != INVALID_ENTITY && sEditorState.selectedEntity == node.folderEntity) {
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	bool open = ImGui::TreeNodeEx(label, flags);

	// フォルダのクリック選択
	if (ImGui::IsItemClicked() && node.folderEntity != INVALID_ENTITY) {
		sEditorState.selectedEntity = node.folderEntity;
		registry.AddComponent<Editor::EditSelectedTag>(sEditorState.selectedEntity);
	}

	// パスの計算（親のパス + 自分の名前）
	std::string fullPath = currentPath;
	if (!node.name.empty()) {
		fullPath = currentPath.empty() ? node.name : (currentPath + "/" + node.name);
	}

	// --- コンテキストメニュー ---
	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::MenuItem("Create Folder")) {
			CreateFolder(registry, "NewFolder", fullPath);
		}
		if (ImGui::MenuItem("Generate newEntity")) {
			Entity e = registry.GenerateEntity();
			auto* tag = registry.AddComponent<Editor::EditTag>(e);
			tag->name = "newEntity";
			tag->path = fullPath + "/" + tag->name;
			registry.AddComponent<Editor::EditSelectedTag>(e);
			Editor::EditorCommandOperator::AddCommand(std::make_unique<Command::InstantiateEntityCommand>(registry, e));
		}
		if (node.folderEntity != INVALID_ENTITY) {
			if (ImGui::MenuItem("Delete Folder")) {
				Editor::EditorCommandOperator::AddCommand(std::make_unique<Command::DeleteEntityCommand>(registry, node.folderEntity));
				registry.DestroyEntity(node.folderEntity);
				// ※ 本来は中身の path も書き換えるか、一緒に消す処理が必要
			}
		}
		ImGui::EndPopup();
	}

	// --- ドロップ受け取り ---
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY")) {
			ECS::Entity dropped = *(ECS::Entity*)payload->Data;
			auto* tag = registry.GetComponent<Editor::EditTag>(dropped);
			if (tag) {
				// 移動先: fullPath (フォルダ内) + 自分の名前
				tag->path = fullPath.empty() ? tag->name : (fullPath + "/" + tag->name);
			}
		}
		ImGui::EndDragDropTarget();
	}

	if (open) {
		// 子フォルダ
		for (auto& [name, child] : node.children) {
			DrawFolderNode(child, registry, fullPath);
		}
		// 所属エンティティ
		for (auto e : node.entities) {
			DrawEntityItem(registry, e);
		}
		ImGui::TreePop();
	}
#else 
	static_cast<void>(node);
	static_cast<void>(registry);
	static_cast<void>(currentPath);

#endif // USE_IMGUI

}
void EditSystem::DrawEntityItem(ECS::Registry& registry, ECS::Entity e) {
#ifdef USE_IMGUI


	auto* tag = registry.GetComponent<Editor::EditTag>(e);
	bool selected = (sEditorState.selectedEntity == e);

	// 選択可能な行
	if (ImGui::Selectable(tag->name.c_str(), selected)) {
		sEditorState.selectedEntity = e;
		registry.AddComponent<Editor::EditSelectedTag>(e);
	}

	// ドラッグ開始（ENTITY ペイロード）
	if (ImGui::BeginDragDropSource()) {
		ImGui::SetDragDropPayload("ENTITY", &e, sizeof(ECS::Entity));
		ImGui::Text("%s", tag->name.c_str());
		ImGui::EndDragDropSource();
	}

	// 右クリックメニュー（個別エンティティ用）
	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::MenuItem("Rename")) {
			// 簡易リネーム処理（ダイアログ等に置き換えてください）
			ImGui::OpenPopup("RenameEntityPopup");
		}
		if (ImGui::MenuItem("Delete")) {
			// 削除処理
			Editor::EditorCommandOperator::AddCommand(std::make_unique<Command::DeleteEntityCommand>(registry, e));
			registry.DestroyEntity(e);
			ImGui::EndPopup();
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

#else
	static_cast<void>(registry);
	static_cast<void>(e);
#endif // USE_IMGUI
}


void EditSystem::DrawAddComponentMenu(Registry& registry, Entity entity) {
#ifdef USE_IMGUI
	if (ImGui::Button("Add Component")) {
		ImGui::OpenPopup("AddComponentPopup");
	}

	if (ImGui::BeginPopup("AddComponentPopup")) {

		for (auto& typeInfo : NoEngine::ComponentRegistry::GetAll()) {

			// すでに持っている Component はスキップ
			if (registry.Has(typeInfo.typeId, entity))
				continue;

			if (ImGui::MenuItem(typeInfo.name.c_str())) {
				// adder を使って Component を追加
				typeInfo.adder(registry, entity);
				Editor::EditorCommandOperator::AddCommand(std::make_unique<Command::AddComponentCommand>(registry, entity, typeInfo.typeId));
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

void EditSystem::CreateFolder(ECS::Registry& registry, const std::string& name, const std::string& parentPath) {
	ECS::Entity folder = registry.GenerateEntity();

	auto* tag = registry.AddComponent<Editor::EditTag>(folder);
	tag->name = name;

	if (parentPath.empty())
		tag->path = name;
	else
		tag->path = parentPath + "/" + name;

	registry.AddComponent<FolderTag>(folder);
	Editor::EditorCommandOperator::AddCommand(std::make_unique<Command::InstantiateEntityCommand>(registry, folder));
}

void EditSystem::AddEntityToFolder(FolderNode& root, ECS::Registry& registry, Entity e) {
	auto* tag = registry.GetComponent<Editor::EditTag>(e);
	bool isFolder = registry.Has<FolderTag>(e);

	// パスを分割（例: "Parent/Sub/MyEntity" -> ["Parent", "Sub", "MyEntity"]）
	std::vector<std::string> segments;
	{
		std::stringstream ss(tag->path);
		std::string seg;
		while (std::getline(ss, seg, '/')) {
			if (!seg.empty()) segments.push_back(seg);
		}
	}

	FolderNode* current = &root;

	if (isFolder) {
		// フォルダーの場合：パスの全セグメントを辿り、最後のノードに Entity を紐付ける
		for (const auto& name : segments) {
			current = &current->children[name];
			current->name = name;
		}
		current->folderEntity = e;
	} else {
		// 通常のエンティティの場合：最後のセグメントを除いた場所まで辿り、そこに push_back
		// ※ tag->path が "Folder/EntityName" なら、segments[0] がフォルダ名
		for (size_t i = 0; i + 1 < segments.size(); ++i) {
			current = &current->children[segments[i]];
			current->name = segments[i];
		}
		current->entities.push_back(e);
	}
}
}
}