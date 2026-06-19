#include "stdafx.h"
#include "PrefabSerializer.h"
#include "SceneSerializer.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI


namespace NoEngine {
using namespace ECS;

namespace Editor {
namespace {
struct PrefabInfo {
	std::string name;
	std::string path;
};
std::vector<PrefabInfo> sPrefabs;

const std::string sFilePath = "resources/game/Prefabs";

}

void SavePreset(ECS::Registry& registry, ECS::Entity e) {
	if (!registry.Has<Editor::EditTag>(e)) {
		LogError("プリセットを保存するエンティティに有効な名前が付けられていません。");
		return;
	}
	std::filesystem::create_directory(sFilePath);
	nlohmann::json j;
	auto* tag = registry.GetComponent<Editor::EditTag>(e);
	std::string presetPath = sFilePath + "/" + tag->name + ".json";
	j = SaveEntityToJson(registry, e);

	std::ofstream ofs(presetPath);
	ofs << j.dump(4);
	LoadPrefabsFromDirectory();
}

Entity InstantiatePreset(ECS::Registry& registry, const std::string& presetPath) {
	nlohmann::json j;
	std::ifstream ifs(presetPath);
	ifs >> j;
	Entity root = registry.GenerateEntity();
	LoadEntityFromJson(registry, root, j);

	return root;
}

void LoadPrefabsFromDirectory() {
	sPrefabs.clear();
	// Prefabフォルダー内をクロール
	for (auto& p : std::filesystem::directory_iterator(sFilePath)) {
		if (!p.is_regular_file()) continue;
		auto ext = p.path().extension().string();
		if (ext != ".json") continue;
		PrefabInfo info;
		info.path = p.path().string();
		info.name = p.path().stem().string();
		
		sPrefabs.push_back(std::move(info));
	}
}

void DrawPrefabWindow(ECS::Registry& registry) {
#ifdef USE_IMGUI
	ImGui::Begin("Prefabs");
	for (auto& prefab : sPrefabs) {
		ImGui::Selectable(prefab.name.c_str());
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
			// ペイロードは null 終端文字列を渡す
			ImGui::SetDragDropPayload("PREFAB_PATH", prefab.path.c_str(), prefab.path.size() + 1);
			ImGui::Text("Place %s", prefab.name.c_str());
			ImGui::EndDragDropSource();
		}
	}
	static_cast<void>(registry);
	ImGui::End();
#else
	static_cast<void>(registry);
#endif // USE_IMGUI

}
}
}
