#include "stdafx.h"
#include "AssetManager.h"

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

std::string AssetManager::GetFilePathFromAddressableName(const std::string& addressableName) {
	if (sAddressable.find(addressableName) == sAddressable.end()) {
		assert(0 && "指定されたAddressableNameが見つかりません！");
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
}