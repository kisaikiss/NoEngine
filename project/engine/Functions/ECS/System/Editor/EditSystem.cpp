#include "EditSystem.h"
#include "engine/Editor/EntityHierarchyUtils.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"
#include "engine/Editor/DataDriven/PrefabSerializer.h"
#include "engine/Editor/ReflectionMacros.h"
#include "engine/Editor/EditorCommandOperator.h"
#include "engine/Functions/Scene/SceneNameComponent.h"
#include "engine/Functions/Command/EditCommand/InstantiateEntitiesCommand.h"

#include "engine/Functions/Input/input.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

#include <unordered_map>

namespace {
const std::string sDirectoryPath = "resources/game/Scenes/";
}

namespace NoEngine {
namespace ECS {

void EditSystem::Update(Registry& registry, float deltaTime) {
	if (!FirstLoaded_) {
		LoadFile(registry);

		// Prefab一覧(DrawPrefabWindow / AssetBrowserWindow)からの「編集」要求を
		// EditSystemが受け取れるようにコールバックを登録する
		Editor::SetEditPrefabCallback([this](ECS::Registry& reg, const std::string& prefabPath) {
			prefabEditorPanel_.Begin(reg, prefabPath);
			});

		FirstLoaded_ = true;
	}
#ifdef USE_IMGUI
	EnsureUniqueEditTagNames(registry);
	DrawMenuBar(registry);

	hierarchyPanel_.Draw(registry);
	inspectorPanel_.Draw(registry);
	prefabEditorPanel_.Draw(registry);

	HandleGlobalShortcuts(registry, deltaTime);
#else
	static_cast<void>(registry);
	static_cast<void>(deltaTime);
#endif // USE_IMGUI
}

void EditSystem::DrawMenuBar(Registry& registry) {
#ifdef USE_IMGUI
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
#else
	static_cast<void>(registry);
#endif // USE_IMGUI
}

void EditSystem::HandleGlobalShortcuts(Registry& registry, float deltaTime) {
#ifdef USE_IMGUI
	// このフレームでEditSelectedTagが付いているEntityを代表選択として扱う。
	// Hierarchy以外（ビューポート上のクリック等）で選択が変わるケースにも対応するため、
	// 特定のパネルの内部状態ではなく、常にECS側(EditSelectedTag)を正として読み直す。


	// 削除（子孫ごと消す。右クリックメニューのDeleteと同じDeleteEntitiesWithUndoを使うことで、
	// Undo/Redoが子孫にも効くことと、削除範囲そのものを両経路で一致させることを保証する）
	if (Input::Keyboard::IsTrigger(VK_DELETE)) {
		Entity selected = SelectedNow(registry);
		if (!ImGui::IsAnyItemActive() && selected != ECS::INVALID_ENTITY) {
			std::vector<Entity> subtree;
			Editor::CollectSubtreeEntities(registry, selected, subtree);

			LogInfo("DestroyEntity name : " + registry.GetComponent<Editor::EditTag>(selected)->name
				+ " (" + std::to_string(subtree.size()) + " entities)");

			for (auto e : subtree) {
				hierarchyPanel_.ClearSelectionOf(e);
			}
			Editor::DeleteEntitiesWithUndo(registry, subtree);
		}
	}

	// コピーやペーストを毎フレーム呼ばないためのインターバル計算
	if (timeInterval_ > 0.0f) {
		timeInterval_ -= deltaTime;
	}

	// コピー（選択エンティティ＋その子孫を全て再帰的にコピーする）
	if (Input::Keyboard::IsPress(VK_LCONTROL) && Input::Keyboard::IsPress('C')) {
		Entity selected = SelectedNow(registry);
		if (selected != ECS::INVALID_ENTITY && selected != prefabEditorPanel_.GetEditingEntity()) {
			if (!ImGui::IsAnyItemActive() && timeInterval_ <= 0.0f) {
				clipboard_.Copy(registry, selected);
				static constexpr float kIntervalTime = 0.3f;
				timeInterval_ = kIntervalTime;
			}
		}
	}

	// ペースト（コピーした親子関係をそのまま新しいEntityとして復元する）
	if (Input::Keyboard::IsPress(VK_LCONTROL) && Input::Keyboard::IsPress('V')) {
		if (timeInterval_ <= 0.0f) {
			if (!ImGui::IsAnyItemActive() && clipboard_.HasContent()) {
				std::vector<Entity> pasted = clipboard_.Paste(registry);

				// 1回のコマンドにまとめて記録することで、1回のUndoで貼り付けた全Entity
				// （子孫含む）をまとめて取り消せるようにする。
				if (!pasted.empty()) {
					Editor::EditorCommandOperator::AddCommand(std::make_unique<Command::InstantiateEntitiesCommand>(registry, pasted));
				}

				// コピー元のルートに対応する新規Entityだけを選択状態にする
				if (!pasted.empty()) {
					registry.AddComponent<Editor::EditSelectedTag>(pasted.front());
				}

				LogInfo("PasteObject : " + std::to_string(pasted.size()) + " entities");
				static constexpr float kIntervalTime = 1.f;
				timeInterval_ = kIntervalTime;
			}
		}
	}
#else
	static_cast<void>(registry);
	static_cast<void>(deltaTime);
#endif // USE_IMGUI
}

Entity EditSystem::SelectedNow(Registry& registry) {
	Entity result = ECS::INVALID_ENTITY;

	auto view = registry.View<Editor::EditTag, Editor::EditSelectedTag>();
	auto it = view.begin();
	if (it != view.end()) {
		result = *it;
	}
	return result;
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
	file.close();
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
			continue;
		}

		// 複数ある場合、最初に出現したものは元名を使い、以降に suffix を付ける方針にする
		int idx = nextIndex[orig];
		if (idx == 1) {
			nextIndex[orig] = 2;
			continue;
		}

		// suffix を付ける
		std::string newName;
		newName.reserve(orig.size() + 8);
		while (true) {
			newName = orig;
			newName.push_back('_');
			newName += std::to_string(idx);

			if (nextIndex.find(newName) == nextIndex.end()) {
				nextIndex[newName] = 1;
				nextIndex[orig] = idx + 1;
				break;
			}
			++idx;
		}

		auto* tag = registry.GetComponent<Editor::EditTag>(e);
		if (tag) {
			tag->name = std::move(newName);
		}
	}
}

}
}