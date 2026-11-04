#include "PrefabEditorPanel.h"
#include "engine/Editor/Components/ComponentUI.h"
#include "engine/Editor/DataDriven/PrefabSerializer.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"
#include "engine/Editor/Panels/InspectorPanel.h"
#include "engine/Editor/Commands/EditorCommandOperator.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

#include <fstream>

namespace NoEngine {
namespace Editor {

void PrefabEditorPanel::Begin(const std::string& prefabPath) {
	// 読み込みに成功してから切り替え、シーンのRegistryにはEntityを生成しない。
	auto editingRegistry = std::make_shared<ECS::Registry>();
	const auto entity = Editor::LoadPrefabForEditing(*editingRegistry, prefabPath);
	End();
	editingRegistry_ = std::move(editingRegistry);
	prefabEntity_ = entity;
	prefabPath_ = prefabPath;
}

void PrefabEditorPanel::End() {
	editingRegistry_.reset();
	prefabEntity_ = ECS::INVALID_ENTITY;
	prefabPath_.clear();
}

void PrefabEditorPanel::Draw(ECS::Registry& sceneRegistry) {
#ifdef USE_IMGUI
	if (prefabEntity_ == ECS::INVALID_ENTITY) return;

	ImGui::Begin("Prefab Editor");
	ImGui::TextDisabled("%s", prefabPath_.c_str());
	ImGui::Separator();

	{
		ScopedEditorRegistry scope(editingRegistry_);
		DrawComponentUI(*editingRegistry_, prefabEntity_);
		InspectorPanel::DrawAddComponentMenu(*editingRegistry_, prefabEntity_);
	}
	ImGui::Separator();
	if (ImGui::Button("Save & Apply")) {
		// 編集結果をPrefabファイルへ書き戻す
		nlohmann::json prefabJson = Editor::SaveEntityToJson(*editingRegistry_, prefabEntity_);

		std::ofstream ofs(prefabPath_);
		ofs << prefabJson.dump(4);
		ofs.close();

		// 既にシーンに配置済みの、同じPrefab由来のEntityへ変更を反映する
		Editor::ApplyPrefabToInstances(sceneRegistry, prefabPath_, prefabJson);

		Editor::LoadPrefabsFromDirectory();
	}
	ImGui::SameLine();
	if (ImGui::Button("Close")) {
		End();
	}

	ImGui::End();
#else
	static_cast<void>(sceneRegistry);
#endif // USE_IMGUI
}

}
}
