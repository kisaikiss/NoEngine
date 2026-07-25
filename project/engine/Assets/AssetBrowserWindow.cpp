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

const std::filesystem::path sResourcesRoot = "resources";

FolderNode sRootFolder;
std::string sSelectedFolder; // "" = ルート直下を表示

char sSearchBuf[128] = {};

// 指定パスの親ディレクトリを resources/ からの相対パスで返す
std::string GetFolderPathOf(const std::string& fullPath) {
	std::filesystem::path parent = std::filesystem::path(fullPath).parent_path();
	return std::filesystem::relative(parent, sResourcesRoot).generic_string();
}

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
		item.folderPath = GetFolderPathOf(asset.metaFilePath);
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
		item.folderPath = GetFolderPathOf(prefab.path);
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

// resources/ 以下のディレクトリ階層を再帰的に構築
void BuildFolderTreeRecursive(FolderNode& node, const std::filesystem::path& dir) {
	std::vector<std::filesystem::path> subDirs;
	for (auto& entry : std::filesystem::directory_iterator(dir)) {
		if (entry.is_directory()) subDirs.push_back(entry.path());
	}
	std::sort(subDirs.begin(), subDirs.end());

	for (auto& sub : subDirs) {
		FolderNode child;
		child.name = sub.filename().string();
		child.relativePath = std::filesystem::relative(sub, sResourcesRoot).generic_string();
		BuildFolderTreeRecursive(child, sub);
		node.children.push_back(std::move(child));
	}
}

void BuildFolderTree() {
	sRootFolder = FolderNode{};
	sRootFolder.name = "resources";
	sRootFolder.relativePath = "";
	BuildFolderTreeRecursive(sRootFolder, sResourcesRoot);
}

// 再帰的にツリーノードを描画。クリックで sSelectedFolder を切り替える
void DrawFolderNode(const FolderNode& node) {
#ifdef USE_IMGUI
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
	if (node.children.empty()) flags |= ImGuiTreeNodeFlags_Leaf;
	if (sSelectedFolder == node.relativePath) flags |= ImGuiTreeNodeFlags_Selected;

	// IDには "##" + relativePath を使い、空文字列でも必ずウィンドウIDと衝突しない形にする
	std::string treeId = "##" + node.relativePath;
	bool open = ImGui::TreeNodeEx(treeId.c_str(), flags, "%s", node.name.c_str());
	if (ImGui::IsItemClicked()) {
		sSelectedFolder = node.relativePath;
	}
	if (open) {
		for (auto& child : node.children) {
			DrawFolderNode(child);
		}
		ImGui::TreePop();
	}
#else
	static_cast<void>(node);
#endif // USE_IMGUI
}

} // namespace

void RefreshAssetBrowser() {
	AssetManager::CreateMetaFileForAllFiles();
	AssetManager::CreateEditorDataList();
	LoadPrefabsFromDirectory();
	RebuildItemList();
	BuildFolderTree();
}

void DrawAssetBrowserFolderWindow() {
#ifdef USE_IMGUI
	ImGui::SetNextWindowSizeConstraints(ImVec2(180, 200), ImVec2(FLT_MAX, FLT_MAX));
	ImGui::Begin("Folders");

	ImGui::SetNextItemWidth(-1);
	ImGui::InputTextWithHint("##search", "検索...", sSearchBuf, sizeof(sSearchBuf));
	ImGui::Separator();

	ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_OpenOnArrow
		| ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen;
	if (sSelectedFolder == sRootFolder.relativePath) rootFlags |= ImGuiTreeNodeFlags_Selected;

	bool rootOpen = ImGui::TreeNodeEx("##RootFolder", rootFlags, "%s", sRootFolder.name.c_str());
	if (ImGui::IsItemClicked()) sSelectedFolder = sRootFolder.relativePath;
	if (rootOpen) {
		for (auto& child : sRootFolder.children) {
			DrawFolderNode(child);
		}
		ImGui::TreePop();
	}

	ImGui::End();
#endif // USE_IMGUI
}

void DrawAssetBrowserWindow(ECS::Registry& registry) {
#ifdef USE_IMGUI
	ImGui::SetNextWindowSizeConstraints(ImVec2(300, 300), ImVec2(FLT_MAX, FLT_MAX));
	ImGui::Begin("Assets");

	const float footerHeight = sInspectorHeight + 8.0f;
	const float minGridHeight = 80.0f;
	float gridHeight = std::max(ImGui::GetContentRegionAvail().y - footerHeight, minGridHeight);

	// --- グリッド（選択フォルダ内のみ表示） ---
	ImGui::BeginChild("BrowserGrid", ImVec2(0, gridHeight), true);
	{
		bool isSearching = (sSearchBuf[0] != '\0');
		if (isSearching) {
			ImGui::TextDisabled("全フォルダから検索中: \"%s\"", sSearchBuf);
		} else {
			ImGui::TextDisabled("フォルダ: %s", sSelectedFolder.empty() ? "resources" : sSelectedFolder.c_str());
		}

		const float cellSize = 90.0f;
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columns = std::max(1, (int)(panelWidth / cellSize));
		ImGui::Columns(columns, nullptr, false);


		for (int i = 0; i < (int)sItems.size(); ++i) {
			auto& item = sItems[i];

			// 検索中はフォルダ絞り込みをスキップして全件対象にする
			if (!isSearching && item.folderPath != sSelectedFolder) continue;

			if (isSearching &&
				item.displayName.find(sSearchBuf) == std::string::npos) {
				continue;
			}

			ImGui::PushID(i);
			bool selected = (sSelectedIndex == i);
			std::string label = std::string(GetIconLabel(item.type)) + "\n" + item.displayName;
			if (ImGui::Selectable(label.c_str(), selected, 0, ImVec2(cellSize - 10, cellSize - 10))) {
				sSelectedIndex = i;
			}

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
				ImGui::SetDragDropPayload(item.dragPayloadType.c_str(), item.path.c_str(), item.path.size() + 1);
				ImGui::Text("Place %s", item.displayName.c_str());
				ImGui::EndDragDropSource();
			}

			ImGui::PopID();
			ImGui::NextColumn();
		}
		ImGui::Columns(1);
	}
	ImGui::EndChild();

	ImGui::Separator();

	// --- インスペクタ ---
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