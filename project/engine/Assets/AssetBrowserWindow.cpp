#include "stdafx.h"
#include "AssetBrowserWindow.h"
#include "AssetManager.h"
#include "engine/Editor/DataDriven/PrefabSerializer.h"
#include "engine/Assets/Audio/Audio.h"
#include "engine/Utilities/Conversion/ConvertString.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "engine/Assets/Texture/Texture.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Functions/Renderer/RaytracingManager.h"
#include "externals/DirectXTex/DirectXTex.h"
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

#ifdef USE_IMGUI
struct TexturePreview {
	std::unique_ptr<Texture> texture;
	DescriptorHandle slot;
	bool hasSlot = false;
	bool attempted = false;
	size_t width = 0;
	size_t height = 0;
};

std::unordered_map<std::string, TexturePreview> sTexturePreviews;
bool sRefreshPreviews = false;

TexturePreview& GetTexturePreview(const BrowserItem& item) {
	auto& preview = sTexturePreviews[item.sourceFilePath];
	if (preview.attempted) return preview;
	preview.attempted = true;
	if (!preview.hasSlot && !Render::gTextureHeap.HasAvailableSpace(1)) return preview;

	// 元画像を直接読み、ゲーム用のDDS変換やテクスチャキャッシュに影響させない。
	DirectX::ScratchImage source;
	const auto filePath = ConvertString(item.sourceFilePath);
	if (FAILED(DirectX::LoadFromWICFile(filePath.c_str(), DirectX::WIC_FLAGS_FORCE_RGB, nullptr, source))) return preview;
	const auto& metadata = source.GetMetadata();
	preview.width = metadata.width;
	preview.height = metadata.height;
	if (preview.width == 0 || preview.height == 0) return preview;

	const float scale = std::min(1.0f, 256.0f / static_cast<float>(std::max(preview.width, preview.height)));
	const size_t width = std::max(size_t(1), static_cast<size_t>(static_cast<float>(preview.width) * scale));
	const size_t height = std::max(size_t(1), static_cast<size_t>(static_cast<float>(preview.height) * scale));
	DirectX::ScratchImage resized;
	if (FAILED(DirectX::Resize(*source.GetImage(0, 0, 0), width, height, DirectX::TEX_FILTER_DEFAULT, resized))) return preview;
	const DirectX::Image* image = resized.GetImage(0, 0, 0);
	DirectX::ScratchImage converted;
	if (image->format != DXGI_FORMAT_R8G8B8A8_UNORM) {
		if (FAILED(DirectX::Convert(*image, DXGI_FORMAT_R8G8B8A8_UNORM, DirectX::TEX_FILTER_DEFAULT, 0.5f, converted))) return preview;
		image = converted.GetImage(0, 0, 0);
	}

	preview.texture = std::make_unique<Texture>();
	preview.texture->Create2D(image->rowPitch, image->width, image->height, image->format, image->pixels);
	if (!preview.hasSlot) {
		preview.slot = Render::gTextureHeap.Alloc();
		preview.hasSlot = true;
	}
	GraphicsCore::sGraphicsDevice->GetDevice()->CopyDescriptorsSimple(1,
		D3D12_CPU_DESCRIPTOR_HANDLE(preview.slot), preview.texture->GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	return preview;
}

ImVec2 FitPreview(const TexturePreview& preview, float width, float height) {
	const float scale = std::min(width / static_cast<float>(preview.width), height / static_cast<float>(preview.height));
	return ImVec2(static_cast<float>(preview.width) * scale, static_cast<float>(preview.height) * scale);
}
#endif

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
		item.path = asset.currentAddressableName.c_str();
		item.metaFilePath = asset.metaFilePath;
		item.sourceFilePath = (std::filesystem::path(asset.metaFilePath).parent_path() / asset.sourceFile).generic_string();
		item.type = AssetManager::GetAssetTypeFromExtension(asset.sourceFile);
		item.dragPayloadType = item.type == EBrowserItemType::Model ? "ASSET_MODEL_NAME"
			: item.type == EBrowserItemType::Texture ? "ASSET_TEXTURE_NAME" : "ASSET_PATH";
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
	case EBrowserItemType::Font:    return "[Font]";
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

// relativePath に一致するFolderNodeを探す
const FolderNode* FindFolderNode(const FolderNode& node, const std::string& relativePath) {
	if (node.relativePath == relativePath) return &node;
	for (auto& child : node.children) {
		if (const FolderNode* found = FindFolderNode(child, relativePath)) return found;
	}
	return nullptr;
}

// 一つ上の階層のrelativePathを返す（ルート直下なら ""）
std::string GetParentFolderPath(const std::string& relativePath) {
	if (relativePath.empty()) return relativePath;
	return std::filesystem::path(relativePath).parent_path().generic_string();
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
#ifdef USE_IMGUI
	sRefreshPreviews = true;
#endif
	AssetManager::CreateMetaFileForAllFiles();
	AssetManager::CreateEditorDataList();
	LoadPrefabsFromDirectory();
	RebuildItemList();
	BuildFolderTree();
}

void ReleaseAssetBrowserPreviews() {
#ifdef USE_IMGUI
	if (!sTexturePreviews.empty()) {
		GraphicsCore::sCommandListManager.IdleGPU();
		sTexturePreviews.clear();
	}
#endif
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
	if (sRefreshPreviews) {
		// 前フレームの描画完了後、今フレームの画像を登録する前にキャッシュを更新する。
		if (!sTexturePreviews.empty()) GraphicsCore::sCommandListManager.IdleGPU();
		for (auto& entry : sTexturePreviews) {
			entry.second.texture.reset();
			entry.second.attempted = false;
		}
		sRefreshPreviews = false;
	}
	bool refreshRequested = false;
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

		// --- フォルダアイテム（検索中は表示しない） ---
		if (!isSearching) {
			// 親フォルダへ戻る
			if (!sSelectedFolder.empty()) {
				ImGui::PushID("..UpFolder");
				ImGui::Selectable("[Folder]\n..", false, 0, ImVec2(cellSize - 10, cellSize - 10));
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					sSelectedFolder = GetParentFolderPath(sSelectedFolder);
					sSelectedIndex = -1;
				}
				ImGui::PopID();
				ImGui::NextColumn();
			}

			const FolderNode* currentFolder = FindFolderNode(sRootFolder, sSelectedFolder);
			if (currentFolder) {
				for (auto& child : currentFolder->children) {
					ImGui::PushID(child.relativePath.c_str());
					std::string label = "[Folder]\n" + child.name;
					ImGui::Selectable(label.c_str(), false, 0, ImVec2(cellSize - 10, cellSize - 10));
					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
						sSelectedFolder = child.relativePath;
						sSelectedIndex = -1;
					}
					ImGui::PopID();
					ImGui::NextColumn();
				}
			}
		}

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
			const ImVec2 tilePos = ImGui::GetCursorScreenPos();
			std::string label = std::string(GetIconLabel(item.type)) + "\n" + item.displayName;
			const bool isTexture = item.type == EBrowserItemType::Texture;
			if (ImGui::Selectable(isTexture ? "##texture" : label.c_str(), selected, 0, ImVec2(cellSize - 10, cellSize - 10))) {
				sSelectedIndex = i;
			}
			if (isTexture && ImGui::IsItemVisible()) {
				auto& preview = GetTexturePreview(item);
				auto* drawList = ImGui::GetWindowDrawList();
				if (preview.texture) {
					const ImVec2 size = FitPreview(preview, cellSize - 18, cellSize - 34);
					const ImVec2 pos(tilePos.x + (cellSize - 10 - size.x) * 0.5f, tilePos.y + 2);
					drawList->AddImage(static_cast<ImTextureID>(preview.slot.GetGpuPtr()), pos, ImVec2(pos.x + size.x, pos.y + size.y));
				} else {
					drawList->AddText(tilePos, ImGui::GetColorU32(ImGuiCol_TextDisabled), "[Tex]");
				}
				const ImVec4 clip(tilePos.x, tilePos.y, tilePos.x + cellSize - 10, tilePos.y + cellSize - 10);
				drawList->AddText(nullptr, 0.0f, ImVec2(tilePos.x, tilePos.y + cellSize - 30),
					ImGui::GetColorU32(ImGuiCol_Text), item.displayName.c_str(), nullptr, 0.0f, &clip);
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", item.displayName.c_str());
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
		if (item.type == EBrowserItemType::Texture) {
			auto& preview = GetTexturePreview(item);
			if (preview.texture) {
				ImGui::Text("%zu x %zu", preview.width, preview.height);
				ImGui::Image(static_cast<ImTextureID>(preview.slot.GetGpuPtr()),
					FitPreview(preview, std::max(1.0f, ImGui::GetContentRegionAvail().x), 160.0f));
			} else {
				ImGui::TextDisabled("プレビューを読み込めません");
			}
		}

		if (item.type == EBrowserItemType::Prefab) {
			ImGui::TextDisabled("(Prefab)");
			ImGui::SameLine();
			if (ImGui::SmallButton("Edit")) {
				// 実際の編集処理はEditSystem側に委譲する（ComponentUIの編集関数を再利用するため）
				RequestEditPrefab(registry, item.path);
			}
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
					refreshRequested = true;
				}
				ImGui::PopStyleColor();
			}

			// --- 音声の試聴 ---
			if (item.type == EBrowserItemType::Audio) {
				if (ImGui::Button("Play")) {
					std::string filePath = AssetManager::GetFilePathFromAddressableName(item.path);
					if (!filePath.empty()) {
						Asset::SoundLoad(ConvertString(filePath), item.path);
						Asset::SoundPlay(item.path, 1.0f, false);
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Stop")) {
					Asset::SoundCompleteStop(item.path);
				}
			}
		}
	} else {
		ImGui::TextDisabled("アイテムを選択してください");
	}

	sInspectorHeight = ImGui::GetCursorPosY() - inspectorStartY;

	ImGui::Separator();
	if (ImGui::Button("Refresh")) {
		refreshRequested = true;
	}

	static_cast<void>(registry);
	ImGui::End();
	if (refreshRequested) RefreshAssetBrowser();
#else
	static_cast<void>(registry);
#endif
}
} // namespace Editor
} // namespace NoEngine
