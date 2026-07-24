#include "stdafx.h"
#include "AssetBrowserWindow.h"
#include "AssetManager.h"
#include "engine/Editor/DataDriven/PrefabSerializer.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

namespace NoEngine {
namespace Editor {
namespace {

std::vector<BrowserItem> sItems;
int sSelectedIndex = -1;
// 前フレームで実測したインスペクタの高さ（初期値は適当な最小値）
float sInspectorHeight = 60.0f;

// AssetManagerとPrefabSerializerの結果を統合BrowserItemに変換
void RebuildItemList() {
	sItems.clear();

	// --- アセット側 ---
	for (auto& asset : AssetManager::GetEditorAssets()) {
		BrowserItem item;
		item.displayName = asset.currentAddressableName;
		item.path = asset.sourceFile;
		item.metaFilePath = asset.metaFilePath;
		item.dragPayloadType = "ASSET_PATH";
		item.type = AssetManager::GetAssetTypeFromExtension(asset.sourceFile);
		item.currentAddressableName = asset.currentAddressableName;
		strcpy_s(item.nameInputBuffer, asset.currentAddressableName.c_str());
		sItems.push_back(item);
	}

	// --- プレハブ側 ---
	for (auto& prefab : GetPrefabs()) {
		BrowserItem item;
		item.displayName = prefab.name;
		item.path = prefab.path;
		item.dragPayloadType = "PREFAB_PATH";
		item.type = EBrowserItemType::Prefab;
		sItems.push_back(item);
	}
}

const char* GetIconLabel(EBrowserItemType type) {
	switch (type) {
	case EBrowserItemType::Model:   return "[Model]";
	case EBrowserItemType::Texture: return "[Tex]";
	case EBrowserItemType::Audio:   return "[Audio]";
	case EBrowserItemType::Prefab:  return "[Prefab]";
	default:                        return "[?]";
	}
}

} // namespace

void RefreshAssetBrowser() {
	AssetManager::CreateMetaFileForAllFiles();
	AssetManager::CreateEditorDataList();
	LoadPrefabsFromDirectory();
	RebuildItemList();
}

void DrawAssetBrowserWindow(ECS::Registry& registry) {
#ifdef USE_IMGUI
	ImGui::SetNextWindowSizeConstraints(ImVec2(300, 250), ImVec2(FLT_MAX, FLT_MAX));
	ImGui::Begin("Asset Browser");

	static char searchBuf[128] = {};
	ImGui::SetNextItemWidth(200.0f);
	ImGui::InputTextWithHint("##search", "検索...", searchBuf, sizeof(searchBuf));

	// --- グリッド: 残り高さから「前フレームのインスペクタ実測高さ」を引く ---
	const float separatorHeight = 8.0f; // Separator分の余白
	const float minGridHeight = 80.0f;
	float gridHeight = std::max(
		ImGui::GetContentRegionAvail().y - sInspectorHeight - separatorHeight,
		minGridHeight);

	ImGui::BeginChild("BrowserGrid", ImVec2(0, gridHeight), true);
	const float cellSize = 90.0f;
	float panelWidth = ImGui::GetContentRegionAvail().x;
	int columns = std::max(1, (int)(panelWidth / cellSize));
	ImGui::Columns(columns, nullptr, false);

	for (int i = 0; i < (int)sItems.size(); ++i) {
		auto& item = sItems[i];
		if (searchBuf[0] != '\0' &&
			item.displayName.find(searchBuf) == std::string::npos) {
			continue;
		}

		ImGui::PushID(i);
		bool selected = (sSelectedIndex == i);
		std::string label = std::string(GetIconLabel(item.type)) + "\n" + item.displayName;
		if (ImGui::Selectable(label.c_str(), selected, 0, ImVec2(cellSize - 10, cellSize - 10))) {
			sSelectedIndex = i;
		}

		// ドラッグ元は種別ごとにペイロードタイプを分けて維持
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
			ImGui::SetDragDropPayload(item.dragPayloadType.c_str(), item.path.c_str(), item.path.size() + 1);
			ImGui::Text("Place %s", item.displayName.c_str());
			ImGui::EndDragDropSource();
		}

		ImGui::PopID();
		ImGui::NextColumn();
	}
	ImGui::Columns(1);
	ImGui::EndChild();
	ImGui::Separator();

	// --- インスペクタ: Childにせず直接描画し、実際の高さを測る ---
	float inspectorStartY = ImGui::GetCursorPosY();

	if (sSelectedIndex >= 0 && sSelectedIndex < (int)sItems.size()) {
		auto& item = sItems[sSelectedIndex];
		ImGui::Text("Path: %s", item.path.c_str());

		if (item.type == EBrowserItemType::Prefab) {
			ImGui::TextDisabled("(Prefab)");
		} else {
			ImGui::SetNextItemWidth(250.0f);
			if (ImGui::InputText("Addressable", item.nameInputBuffer, sizeof(item.nameInputBuffer))) {
				item.isModified = (item.currentAddressableName != item.nameInputBuffer);
			}
			if (item.isModified) {
				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
				if (ImGui::Button("Save")) {
					AssetManager::EditorAssetData data;
					data.metaFilePath = item.metaFilePath;
					strcpy_s(data.nameInputBuffer, item.nameInputBuffer);
					AssetManager::SaveMetaFile(data);
					RefreshAssetBrowser();
				}
				ImGui::PopStyleColor();
			}
		}
	} else {
		ImGui::TextDisabled("アイテムを選択してください");
	}

	// 今フレームの実測値を保存 → 次フレームのグリッド高さ計算に使う
	sInspectorHeight = ImGui::GetCursorPosY() - inspectorStartY;

	ImGui::Separator();
	if (ImGui::Button("Refresh")) {
		RefreshAssetBrowser();
	}
	static_cast<void>(registry);
	ImGui::End();
#else
	static_cast<void>(registry);
#endif
}
} // namespace Editor
} // namespace NoEngine