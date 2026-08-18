#include "PrefabEditorPanel.h"
#include "engine/Editor/ComponentUI.h"
#include "engine/Editor/DataDriven/PrefabSerializer.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"
#include "engine/Editor/InspectorPanel.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

#include <fstream>

namespace NoEngine {
namespace Editor {

void PrefabEditorPanel::Begin(ECS::Registry& registry, const std::string& prefabPath) {
	// 既に別のPrefabを編集中だった場合は、その一時Entityを破棄してから切り替える
	End(registry);

	prefabEntity_ = Editor::LoadPrefabForEditing(registry, prefabPath);
	prefabPath_ = prefabPath;
}

void PrefabEditorPanel::End(ECS::Registry& registry) {
	if (prefabEntity_ != ECS::INVALID_ENTITY) {
		registry.DestroyEntity(prefabEntity_);
		prefabEntity_ = ECS::INVALID_ENTITY;
	}
	prefabPath_.clear();
}

void PrefabEditorPanel::Draw(ECS::Registry& registry) {
#ifdef USE_IMGUI
	if (prefabEntity_ == ECS::INVALID_ENTITY) return;

	ImGui::Begin("Prefab Editor");
	ImGui::TextDisabled("%s", prefabPath_.c_str());
	ImGui::Separator();

	DrawComponentUI(registry, prefabEntity_);

	InspectorPanel::DrawAddComponentMenu(registry, prefabEntity_);
	ImGui::Separator();
	if (ImGui::Button("Save & Apply")) {
		// 編集結果をPrefabファイルへ書き戻す
		nlohmann::json prefabJson = Editor::SaveEntityToJson(registry, prefabEntity_);

		std::ofstream ofs(prefabPath_);
		ofs << prefabJson.dump(4);
		ofs.close();

		// 既にシーンに配置済みの、同じPrefab由来のEntityへ変更を反映する
		Editor::ApplyPrefabToInstances(registry, prefabPath_, prefabJson);

		Editor::LoadPrefabsFromDirectory();
	}
	ImGui::SameLine();
	if (ImGui::Button("Close")) {
		End(registry);
	}

	ImGui::End();
#else
	static_cast<void>(registry);
#endif // USE_IMGUI
}

}
}
