#include "stdafx.h"
#include "AssetManager.h"
#ifdef USE_IMGUI
#include "externals/Imgui/imgui.h"
#endif // USE_IMGUI


namespace NoEngine {
namespace {
std::string sAssetFilePass = "resources/";   // resourcesファイルの先頭ディレクトリ
std::string sMetaFileExtensionName = ".nometa"; // 作成するメタファイルの拡張子
// 対応する拡張子
std::list<std::string> sSupportedExtensions;

//メタ情報
struct MetaData {
	std::string filePath;
};

// AddressableNameとメタ情報のリスト
std::unordered_map<std::string, MetaData> sAddressable;

std::vector<AssetManager::EditorAssetData> sEditorAssets;
}

void AssetManager::CreateMetaFileForAllFiles() {
	sSupportedExtensions.clear();
	sSupportedExtensions.push_back(".obj");
	sSupportedExtensions.push_back(".gltf");
	sSupportedExtensions.push_back(".png");
	sSupportedExtensions.push_back(".mp3");
	sSupportedExtensions.push_back(".wav");

	// 指定されたアセットフォルダ以下をクロール
	for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(sAssetFilePass)) {
		// AssetManagerがサポートしているファイルだった
		if (entry.is_regular_file() && IsSupportedFile(entry.path())) {
			// メタファイルが有るか
			std::filesystem::path metafilePath = entry.path();
			metafilePath.replace_filename(entry.path().filename().string() + sMetaFileExtensionName);
			if (std::filesystem::exists(metafilePath) == false) {
				// メタファイルがなかったら新規作成
				std::ofstream metaFile(metafilePath);
				LogInfo("Create Meta File : " + metafilePath.string());
				// メタファイルに書き込むデータの作成
				metaFile << CreateMetaFileForFile(entry.path());
			}
		}
	}
}

void AssetManager::DeleteAllMetaFiles() {
	// Assetフォルダをクロール
	for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(sAssetFilePass)) {
		// Metaファイルじゃなかったら無視
		if (entry.path().extension().string() != sMetaFileExtensionName) { continue; }

		// 削除
		if (std::filesystem::remove(entry.path())) {
			LogInfo("Delete Mata File : " + entry.path().string());
		}
	}
}

void AssetManager::CreateAddressableList() {
	sAddressable.clear();
	for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(sAssetFilePass)) {
		// メタファイルかどうか
		if (entry.is_regular_file() && entry.path().extension().string() == sMetaFileExtensionName) {
			std::ifstream meta(entry.path().string());
			nlohmann::json j;
			meta >> j;

			// AddressableName
			std::string addressable = j["AddressableName"];
			// 今回実行時のファイルパスの作成
			std::string sourceFileName = j["SourceFile"];

			std::filesystem::path onlyDi = entry.path();
			onlyDi.remove_filename();
			std::string directory = onlyDi.relative_path().generic_string();

			if (!directory.empty() && directory.back() != '/') directory += '/';
			std::string filePath = directory + sourceFileName;

			// メタデータから参照ファイル情報の作成
			MetaData metaData;
			metaData.filePath = filePath;

			// Addressableの被りは許さず
			if (sAddressable.find(addressable) != sAddressable.end()) {
				LogError("error! : " + addressable + " This AddressableName is Conflict!  filePath : " + metaData.filePath);
				assert(0 && "AddressableNameが被っています！！ Logファイルを参照して下さい");
			}

			// AddressableNameをキーにしてデータを覚えておく
			sAddressable[addressable] = metaData;
		}
	}
}

void AssetManager::CreateEditorDataList() {
	sEditorAssets.clear();

	// メタファイルをクロールしてエディタ用リストを構築
	for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(sAssetFilePass)) {
		if (entry.is_regular_file() && entry.path().extension().string() == sMetaFileExtensionName) {
			std::ifstream meta(entry.path().string());
			nlohmann::json j;
			meta >> j;

			EditorAssetData data;
			data.metaFilePath = entry.path().string();
			data.sourceFile = j.value("SourceFile", "Unknown");
			data.currentAddressableName = j.value("AddressableName", "Unknown");

			// ImGui用のchar配列にコピー
			strcpy_s(data.nameInputBuffer, data.currentAddressableName.c_str());

			data.isModified = false;
			sEditorAssets.push_back(data);
		}
	}
}

const std::vector<AssetManager::EditorAssetData>& AssetManager::GetEditorAssets() {
	return sEditorAssets;
}

void AssetManager::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("Asset Manager");

	if (ImGui::Button("Refresh Meta Files")) {
		CreateMetaFileForAllFiles(); // 新規ファイルがあればメタを作る
		CreateEditorDataList();            // エディタ情報を再読み込み
	}

	ImGui::Separator();

	// 各アセットの編集UIを描画
	for (auto& asset : sEditorAssets) {
		// 同じ名前のUI要素が被らないようにIDをPush
		ImGui::PushID(asset.metaFilePath.c_str());

		// 元ファイル名の表示 (Read Only)
		ImGui::Text("File: %s", asset.sourceFile.c_str());

		// AddressableNameの編集
		ImGui::SetNextItemWidth(250.0f);
		if (ImGui::InputText("Addressable", asset.nameInputBuffer, sizeof(asset.nameInputBuffer))) {
			// 文字列が変更されたかチェック
			asset.isModified = (asset.currentAddressableName != asset.nameInputBuffer);
		}

		// 変更がある場合のみ Save ボタンを表示
		if (asset.isModified) {
			ImGui::SameLine();
			// ボタンの色を目立つように変更（オプション）
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
			if (ImGui::Button("Save")) {
				SaveMetaFile(asset);
			}
			ImGui::PopStyleColor();
		}

		ImGui::PopID();
		ImGui::Separator();
	}

	ImGui::End();
#endif // USE_IMGUI

}

std::string AssetManager::GetFilePathFromAddressableName(const std::string& addressableName) {
	if (sAddressable.find(addressableName) == sAddressable.end()) {
		LogWarning("指定されたAddressableNameが見つかりません！");
		return "";
	}

	return sAddressable[addressableName].filePath;
}

bool AssetManager::IsSupportedFile(const std::filesystem::path& filePath) {
	for (auto& ext : sSupportedExtensions) {
		if (filePath.extension().string() == ext) { return true; }
	}
	return false;
}

nlohmann::json AssetManager::CreateMetaFileForFile(const std::filesystem::path& srcFile) {
	nlohmann::json j;
	j["SourceFile"] = srcFile.filename().string();
	j["AddressableName"] = AddressableName(srcFile);
	return j;
}

Editor::EBrowserItemType AssetManager::GetAssetTypeFromExtension(const std::filesystem::path& path) {
	static const std::unordered_map<std::string, Editor::EBrowserItemType> table = {
		{".obj",  Editor::EBrowserItemType::Model},
		{".gltf", Editor::EBrowserItemType::Model},
		{".png",  Editor::EBrowserItemType::Texture},
		{".mp3",  Editor::EBrowserItemType::Audio},
		{".wav",  Editor::EBrowserItemType::Audio},
	};
	auto it = table.find(path.extension().string());
	return it != table.end() ? it->second : Editor::EBrowserItemType::Unknown;
}

std::string AssetManager::AddressableName(const std::filesystem::path& srcFile) {
	auto filename = srcFile.filename().string();

	// AddressableNameの指定が有るか
	auto at = filename.find("@");
	if (at != std::string::npos) {
		auto dot = filename.find(".");
		return filename.substr(at + 1, dot - at - 1);
	}
	// Addressableが指定されていない場合、ファイルパスをそのまま
	return srcFile.relative_path().generic_string();

}

void AssetManager::SaveMetaFile(AssetManager::EditorAssetData& data) {
	// 既存のJSONを読み込み
	std::ifstream ifs(data.metaFilePath);
	nlohmann::json j;
	if (ifs.is_open()) {
		ifs >> j;
		ifs.close();
	}

	// 値を上書き
	std::string newName = data.nameInputBuffer;
	j["AddressableName"] = newName;

	// ファイルに書き戻す (インデント付きで見やすく)
	std::ofstream ofs(data.metaFilePath);
	ofs << j.dump(4);
	ofs.close();

	// 状態の更新
	data.currentAddressableName = newName;
	data.isModified = false;
	LogInfo("Update Meta File : " + data.metaFilePath + " -> " + newName);

	// ★ 変更を即座にゲームエンジン側に反映するため、リストを再構築
	CreateAddressableList();
}
}