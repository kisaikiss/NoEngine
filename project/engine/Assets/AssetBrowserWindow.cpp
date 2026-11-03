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
#include <shellapi.h>
#include <shlobj.h>
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Ole32.lib")
#endif // USE_IMGUI

namespace NoEngine {
namespace Editor {
namespace {

std::vector<BrowserItem> sItems;
int sSelectedIndex = -1;

const std::filesystem::path sResourcesRoot = "resources";

FolderNode sRootFolder;
std::string sSelectedFolder; // "" = ルート直下を表示

char sSearchBuf[128] = {};
int sTypeFilter = 0;
int sSortMode = 0;
bool sSortDescending = false;
bool sListView = false;
bool sSearchAllFolders = true;
float sThumbnailSize = 90.0f;

const std::string& GetSourcePath(const BrowserItem& item) {
	return item.type == EBrowserItemType::Prefab ? item.path : item.sourceFilePath;
}

// UTF-8の日本語を維持し、ASCIIの英字とパス区切りだけを正規化する。
std::string NormalizeSearch(std::string text) {
	for (char& c : text) {
		if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
		if (c == '\\') c = '/';
	}
	return text;
}

std::vector<std::string> GetSearchTerms() {
	std::istringstream stream(NormalizeSearch(sSearchBuf));
	std::vector<std::string> terms;
	std::string term;
	while (stream >> term) terms.push_back(term);
	return terms;
}

bool MatchesSearch(const BrowserItem& item, const std::vector<std::string>& terms) {
	const std::string text = NormalizeSearch(item.displayName + "\n" + GetSourcePath(item));
	return std::all_of(terms.begin(), terms.end(), [&](const std::string& term) {
		return text.find(term) != std::string::npos;
	});
}

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
	const auto relative = std::filesystem::relative(parent, sResourcesRoot).generic_string();
	return relative == "." ? "" : relative;
}

// AssetManagerとPrefabSerializerの結果を統合BrowserItemに変換
void RebuildItemList() {
	// 更新で項目の順番が変わっても、同じ素材の選択を維持する。
	const std::string selectedPath = sSelectedIndex >= 0 && sSelectedIndex < static_cast<int>(sItems.size())
		? GetSourcePath(sItems[sSelectedIndex]) : "";
	sSelectedIndex = -1;
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
	for (int i = 0; i < static_cast<int>(sItems.size()); ++i) {
		if (!selectedPath.empty() && GetSourcePath(sItems[i]) == selectedPath) sSelectedIndex = i;
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

#ifdef USE_IMGUI
std::string sBrowserError;
std::string sPreviewPath;
std::string sPlayingPreviewAudio;
std::string sAddressableMetaPath;
char sAddressableName[128] = {};
bool sFocusPreview = false;
bool sFocusAddressable = false;

void StopPreviewAudio() {
	if (!sPlayingPreviewAudio.empty()) {
		Asset::SoundCompleteStop(sPlayingPreviewAudio);
		sPlayingPreviewAudio.clear();
	}
}

void DrawAssetToolWindows(bool& refreshRequested) {
	if (!sPreviewPath.empty()) {
		bool open = true;
		ImGui::SetNextWindowSize(ImVec2(480, 420), ImGuiCond_FirstUseEver);
		if (sFocusPreview) ImGui::SetNextWindowFocus();
		sFocusPreview = false;
		if (ImGui::Begin("アセットプレビュー###AssetPreview", &open)) {
			const auto it = std::find_if(sItems.begin(), sItems.end(), [](const BrowserItem& item) {
				return GetSourcePath(item) == sPreviewPath;
			});
			if (it == sItems.end()) {
				ImGui::TextDisabled("アセットが見つかりません");
			} else {
				ImGui::TextWrapped("%s", it->displayName.c_str());
				ImGui::TextWrapped("%s", GetSourcePath(*it).c_str());
				ImGui::Separator();
				if (it->type == EBrowserItemType::Texture) {
					auto& preview = GetTexturePreview(*it);
					if (preview.texture) {
						ImGui::Text("%zu x %zu", preview.width, preview.height);
						const auto available = ImGui::GetContentRegionAvail();
						ImGui::Image(static_cast<ImTextureID>(preview.slot.GetGpuPtr()),
							FitPreview(preview, std::max(1.0f, available.x), std::max(1.0f, available.y)));
					} else {
						ImGui::TextDisabled("プレビューを読み込めません");
					}
				} else if (it->type == EBrowserItemType::Audio) {
					if (ImGui::Button("Play")) {
						StopPreviewAudio();
						Asset::SoundLoad(ConvertString(it->sourceFilePath), it->path);
						Asset::SoundPlay(it->path, 1.0f, false);
						sPlayingPreviewAudio = it->path;
					}
					ImGui::SameLine();
					if (ImGui::Button("Stop")) StopPreviewAudio();
				}
			}
		}
		ImGui::End();
		if (!open) {
			StopPreviewAudio();
			sPreviewPath.clear();
		}
	}
	if (!sAddressableMetaPath.empty()) {
		bool open = true;
		ImGui::SetNextWindowSize(ImVec2(480, 200), ImGuiCond_FirstUseEver);
		if (sFocusAddressable) ImGui::SetNextWindowFocus();
		sFocusAddressable = false;
		if (ImGui::Begin("Addressable名の変更###AssetAddressableEditor", &open)) {
			// 選択や並び順に依存せず、メタファイルで編集対象を特定する。
			const auto it = std::find_if(sItems.begin(), sItems.end(), [](const BrowserItem& item) {
				return item.metaFilePath == sAddressableMetaPath;
			});
			if (it == sItems.end()) {
				ImGui::TextDisabled("アセットが見つかりません");
			} else {
				ImGui::TextWrapped("%s", it->sourceFilePath.c_str());
				ImGui::SetNextItemWidth(-1);
				ImGui::InputText("##AddressableName", sAddressableName, sizeof(sAddressableName));
				const bool empty = sAddressableName[0] == '\0';
				const bool duplicate = std::any_of(sItems.begin(), sItems.end(), [&](const BrowserItem& item) {
					return item.type != EBrowserItemType::Prefab && item.metaFilePath != sAddressableMetaPath
						&& item.currentAddressableName == sAddressableName;
				});
				if (empty) ImGui::TextDisabled("名前を入力してください");
				if (duplicate) ImGui::TextDisabled("このAddressable名は既に使用されています");
				ImGui::BeginDisabled(empty || duplicate || it->currentAddressableName == sAddressableName);
				if (ImGui::Button("保存")) {
					AssetManager::EditorAssetData data{};
					data.metaFilePath = it->metaFilePath;
					strcpy_s(data.nameInputBuffer, sAddressableName);
					AssetManager::SaveMetaFile(data);
					refreshRequested = true;
					open = false;
				}
				ImGui::EndDisabled();
				ImGui::SameLine();
				if (ImGui::Button("キャンセル")) open = false;
			}
		}
		ImGui::End();
		if (!open) sAddressableMetaPath.clear();
	}
	if (!sBrowserError.empty()) {
		bool open = true;
		if (ImGui::Begin("アセット操作エラー###AssetBrowserError", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::TextUnformatted(sBrowserError.c_str());
			if (ImGui::Button("閉じる")) open = false;
		}
		ImGui::End();
		if (!open) sBrowserError.clear();
	}
}

void OpenSourcePath(const std::filesystem::path& path, bool reveal) {
	std::error_code error;
	auto absolute = std::filesystem::absolute(path, error);
	absolute.make_preferred();
	if (error || !std::filesystem::exists(absolute, error) || error) {
		sBrowserError = "ファイルまたはフォルダが見つかりません";
		return;
	}
	bool succeeded = false;
	if (reveal) {
		// シェルのパスAPIを使い、空白や日本語を含むファイルも選択する。
		PIDLIST_ABSOLUTE itemId = ILCreateFromPathW(absolute.c_str());
		if (itemId) {
			succeeded = SUCCEEDED(SHOpenFolderAndSelectItems(itemId, 0, nullptr, 0));
			ILFree(itemId);
		}
	} else {
		succeeded = reinterpret_cast<INT_PTR>(ShellExecuteW(nullptr, L"open", absolute.c_str(), nullptr, nullptr, SW_SHOWNORMAL)) > 32;
	}
	sBrowserError = succeeded ? "" : "ファイルまたはフォルダを開けませんでした";
}

void DrawFolderContextMenu(const std::string& relativePath) {
	if (ImGui::BeginPopupContextItem()) {
		const auto path = sResourcesRoot / relativePath;
		if (ImGui::MenuItem("エクスプローラーで開く")) OpenSourcePath(path, false);
		if (ImGui::MenuItem("パスをコピー")) ImGui::SetClipboardText(path.generic_string().c_str());
		ImGui::EndPopup();
	}
}

void DrawAssetContextMenu(BrowserItem& item, int index, ECS::Registry& registry) {
	if (ImGui::BeginPopupContextItem()) {
		sSelectedIndex = index;
		const auto& source = GetSourcePath(item);
		if ((item.type == EBrowserItemType::Texture || item.type == EBrowserItemType::Audio)
			&& ImGui::MenuItem("プレビューを開く")) {
			if (sPreviewPath != source) StopPreviewAudio();
			sPreviewPath = source;
			sFocusPreview = true;
		}
		if (item.type != EBrowserItemType::Prefab && ImGui::MenuItem("Addressable名を変更")) {
			sAddressableMetaPath = item.metaFilePath;
			strcpy_s(sAddressableName, item.currentAddressableName.c_str());
			sFocusAddressable = true;
		}
		ImGui::Separator();
		if (ImGui::MenuItem("エクスプローラーで表示")) OpenSourcePath(ConvertString(source), true);
		if (ImGui::MenuItem("元ファイルを開く")) OpenSourcePath(ConvertString(source), false);
		ImGui::Separator();
		if (ImGui::MenuItem("Addressable名をコピー", nullptr, false, item.type != EBrowserItemType::Prefab))
			ImGui::SetClipboardText(item.currentAddressableName.c_str());
		if (ImGui::MenuItem("パスをコピー")) ImGui::SetClipboardText(source.c_str());
		if (item.type == EBrowserItemType::Prefab && ImGui::MenuItem("Prefabを編集"))
			RequestEditPrefab(registry, item.path);
		ImGui::EndPopup();
	}
}
#endif

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
	DrawFolderContextMenu(node.relativePath);
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
	if (!FindFolderNode(sRootFolder, sSelectedFolder)) sSelectedFolder.clear();
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
	ImGui::InputTextWithHint("##search", "名前・ファイル名・パスで検索", sSearchBuf, sizeof(sSearchBuf));
	if (ImGui::Button("検索をクリア")) sSearchBuf[0] = '\0';
	ImGui::Checkbox("検索時は全フォルダを対象", &sSearchAllFolders);
	ImGui::TextDisabled("空白区切りでAND検索 / 英字の大小を区別しません");
	ImGui::Separator();

	ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_OpenOnArrow
		| ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen;
	if (sSelectedFolder == sRootFolder.relativePath) rootFlags |= ImGuiTreeNodeFlags_Selected;

	bool rootOpen = ImGui::TreeNodeEx("##RootFolder", rootFlags, "%s", sRootFolder.name.c_str());
	if (ImGui::IsItemClicked()) sSelectedFolder = sRootFolder.relativePath;
	DrawFolderContextMenu("");
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
	if (!ImGui::Begin("Assets", nullptr, ImGuiWindowFlags_MenuBar)) {
		ImGui::End();
		DrawAssetToolWindows(refreshRequested);
		if (refreshRequested) RefreshAssetBrowser();
		return;
	}

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("表示")) {
			if (ImGui::MenuItem("サムネイル", nullptr, !sListView)) sListView = false;
			if (ImGui::MenuItem("一覧", nullptr, sListView)) sListView = true;
			ImGui::Separator();
			ImGui::SetNextItemWidth(160.0f);
			ImGui::SliderFloat("サイズ", &sThumbnailSize, 64.0f, 192.0f, "%.0f");
			if (ImGui::BeginMenu("並び順")) {
				const char* labels[] = { "名前", "種類", "パス" };
				for (int i = 0; i < 3; ++i) {
					if (ImGui::MenuItem(labels[i], nullptr, sSortMode == i)) sSortMode = i;
				}
				ImGui::Separator();
				ImGui::MenuItem("降順", nullptr, &sSortDescending);
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("種類")) {
			const char* labels[] = { "すべて", "Model", "Texture", "Audio", "Font", "Prefab", "Unknown" };
			for (int i = 0; i < 7; ++i) {
				if (ImGui::MenuItem(labels[i], nullptr, sTypeFilter == i)) sTypeFilter = i;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("操作")) {
			if (ImGui::MenuItem("Refresh")) refreshRequested = true;
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	const auto searchTerms = GetSearchTerms();
	const bool isSearching = !searchTerms.empty();
	const bool globalSearch = isSearching && sSearchAllFolders;
	std::vector<int> visibleItems;
	for (int i = 0; i < static_cast<int>(sItems.size()); ++i) {
		const auto& item = sItems[i];
		if (!globalSearch && item.folderPath != sSelectedFolder) continue;
		if (sTypeFilter != 0 && static_cast<int>(item.type) != sTypeFilter - 1) continue;
		if (MatchesSearch(item, searchTerms)) visibleItems.push_back(i);
	}
	// 元配列を並べ替えず、選択とドラッグ対象の対応を保つ。
	std::sort(visibleItems.begin(), visibleItems.end(), [](int left, int right) {
		const auto& a = sItems[left];
		const auto& b = sItems[right];
		int order = 0;
		if (sSortMode == 1) order = static_cast<int>(a.type) - static_cast<int>(b.type);
		if (sSortMode == 2) order = NormalizeSearch(GetSourcePath(a)).compare(NormalizeSearch(GetSourcePath(b)));
		if (order == 0) order = NormalizeSearch(a.displayName).compare(NormalizeSearch(b.displayName));
		if (order == 0) order = GetSourcePath(a).compare(GetSourcePath(b));
		if (order == 0) order = left < right ? -1 : (left > right ? 1 : 0);
		return sSortDescending ? order > 0 : order < 0;
	});

	// メニューバー以外の領域全体をアセット一覧に割り当てる。
	ImGui::BeginChild("BrowserGrid", ImVec2(0, 0));
	{
		const float cellSize = sThumbnailSize;
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columns = sListView ? 1 : std::max(1, (int)(panelWidth / cellSize));
		ImGui::Columns(columns, nullptr, false);
		const ImVec2 itemSize = sListView ? ImVec2(0, ImGui::GetTextLineHeightWithSpacing() + 4.0f)
			: ImVec2(std::max(1.0f, std::min(cellSize - 10, panelWidth)), cellSize - 10);
		std::string nextFolder = sSelectedFolder;

		// --- フォルダアイテム（検索中は表示しない） ---
		if (!isSearching) {
			// 親フォルダへ戻る
			if (!sSelectedFolder.empty()) {
				ImGui::PushID("..UpFolder");
				ImGui::Selectable(sListView ? "[Folder] .." : "[Folder]\n..", false, 0, itemSize);
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					nextFolder = GetParentFolderPath(sSelectedFolder);
				}
				ImGui::PopID();
				ImGui::NextColumn();
			}

			const FolderNode* currentFolder = FindFolderNode(sRootFolder, sSelectedFolder);
			if (currentFolder) {
				for (auto& child : currentFolder->children) {
					ImGui::PushID(child.relativePath.c_str());
					std::string label = std::string(sListView ? "[Folder] " : "[Folder]\n") + child.name;
					ImGui::Selectable(label.c_str(), false, 0, itemSize);
					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
						nextFolder = child.relativePath;
					}
					DrawFolderContextMenu(child.relativePath);
					ImGui::PopID();
					ImGui::NextColumn();
				}
			}
		}

		for (int i : visibleItems) {
			auto& item = sItems[i];

			ImGui::PushID(i);
			bool selected = (sSelectedIndex == i);
			const ImVec2 tilePos = ImGui::GetCursorScreenPos();
			const bool isTexture = item.type == EBrowserItemType::Texture;
			if (ImGui::Selectable("##asset", selected, 0, itemSize)) {
				sSelectedIndex = i;
			}
			if (ImGui::IsItemVisible()) {
				auto* drawList = ImGui::GetWindowDrawList();
				const float tileWidth = sListView ? ImGui::GetContentRegionAvail().x : itemSize.x;
				const ImVec4 clip(tilePos.x, tilePos.y, tilePos.x + tileWidth, tilePos.y + itemSize.y);
				if (sListView) {
					const std::string text = std::string(GetIconLabel(item.type)) + " " + item.displayName + "    " + GetSourcePath(item);
					drawList->AddText(nullptr, 0.0f, tilePos, ImGui::GetColorU32(ImGuiCol_Text), text.c_str(), nullptr, 0.0f, &clip);
				} else {
					if (isTexture) {
						auto& preview = GetTexturePreview(item);
						if (preview.texture) {
							const ImVec2 size = FitPreview(preview, std::max(1.0f, tileWidth - 8), cellSize - 34);
							const ImVec2 pos(tilePos.x + (tileWidth - size.x) * 0.5f, tilePos.y + 2);
							drawList->AddImage(static_cast<ImTextureID>(preview.slot.GetGpuPtr()), pos, ImVec2(pos.x + size.x, pos.y + size.y));
						} else {
							drawList->AddText(tilePos, ImGui::GetColorU32(ImGuiCol_TextDisabled), "[Tex]");
						}
					} else {
						drawList->AddText(nullptr, 0.0f, tilePos, ImGui::GetColorU32(ImGuiCol_TextDisabled), GetIconLabel(item.type), nullptr, 0.0f, &clip);
					}
					drawList->AddText(nullptr, 0.0f, ImVec2(tilePos.x, tilePos.y + cellSize - 30),
						ImGui::GetColorU32(ImGuiCol_Text), item.displayName.c_str(), nullptr, 0.0f, &clip);
				}
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\n%s\n%s", item.displayName.c_str(), GetIconLabel(item.type), GetSourcePath(item).c_str());

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
				ImGui::SetDragDropPayload(item.dragPayloadType.c_str(), item.path.c_str(), item.path.size() + 1);
				ImGui::Text("Place %s", item.displayName.c_str());
				ImGui::EndDragDropSource();
			}
			DrawAssetContextMenu(item, i, registry);

			ImGui::PopID();
			ImGui::NextColumn();
		}
		ImGui::Columns(1);
		if (visibleItems.empty()) ImGui::TextDisabled("該当するアセットはありません");
		if (nextFolder != sSelectedFolder) {
			sSelectedFolder = nextFolder;
			sSelectedIndex = -1;
		}
	}
	ImGui::EndChild();

	ImGui::End();
	DrawAssetToolWindows(refreshRequested);
	if (refreshRequested) RefreshAssetBrowser();
#else
	static_cast<void>(registry);
#endif
}
} // namespace Editor
} // namespace NoEngine
