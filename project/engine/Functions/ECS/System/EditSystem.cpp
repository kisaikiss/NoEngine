#include "EditSystem.h"
#include "engine/Editor/EditUtils.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"
#include "engine/Functions/Debug/Logger/Log.h"
#include "engine/Functions/Scene/SceneNameComponent.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

namespace {
const std::string skDirectoryPath = "resources/game/Scenes/";
}

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
	if (sEditorState.selectedEntity != INVALID_ENTITY) {
		auto* tag = registry.GetComponent<Editor::EditTag>(sEditorState.selectedEntity);
		if (tag) {
			ImGui::BeginChild(tag->name.c_str());
			ImGui::Text(tag->name.c_str());
			DrawComponentUI(registry, sEditorState.selectedEntity);
			ImGui::EndChild();
		}
	}
	ImGui::End();


#else
	static_cast<void>(registry);
#endif // USE_IMGUI
}

void EditSystem::SaveFile(Registry& registry, nlohmann::json j) {
	std::filesystem::path dir(skDirectoryPath);
	if (!std::filesystem::exists(dir)) {
		std::filesystem::create_directory(dir);
	}

	std::string sceneName;
	auto view = registry.View<SceneNameComponent>();
	for (auto entity : view) {
		auto* nameComp = registry.GetComponent<SceneNameComponent>(entity);
		sceneName = nameComp->GetName();
	}
	std::string filePath = skDirectoryPath + sceneName + ".json";
	std::ofstream file(filePath);
	if (!file.is_open()) {
		Log::DebugPrint("Failed to open file for writing", VerbosityLevel::kError);
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
	std::string filePath = skDirectoryPath + sceneName + ".json";

	std::ifstream file(filePath);
	if (!file.is_open()) {
		Log::DebugPrint("Failed to open scene file:" + filePath, VerbosityLevel::kError);
		return;
	}

	nlohmann::json scene;
	file >> scene;

	Editor::LoadScene(registry, scene);
}

void EditSystem::DrawHierarchyWindow(Registry& registry) {
#ifdef USE_IMGUI
	ImGui::Begin("Hierarchy");

	auto view = registry.View<Editor::EditTag>();

	for (auto e : view) {
		auto* tag = registry.GetComponent<Editor::EditTag>(e);
		if (!tag->isDrawHierarchy) continue;

		bool selected = (sEditorState.selectedEntity == e);
		if (ImGui::Selectable(tag->name.c_str(), selected)) {
			sEditorState.selectedEntity = e;
		}
	}

	ImGui::End();
#else
	static_cast<void>(registry);
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

}
}