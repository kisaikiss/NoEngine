#pragma once
#include "externals/nlohmann/json.hpp"

namespace NoEngine {
class AssetManager {
public:

	/// <summary>
	/// resourcesフォルダ以下をクロールして、metaファイルを更新していく
	/// </summary>
	static void CreateMetaFileForAllFiles();

	/// <summary>
	/// 作成したmetaファイルを全削除(危険)
	/// </summary>
	static void DeleteAllMetaFiles();

	// Metaファイルを探してApplicationで使用するリストを作成する
	static void CreateAddressableList();

	// AddressableNameを指定してファイルパスの作成
	static std::string GetFilePathFromAddressableName(const std::string& addressableName);
private:
	/// <summary>
	/// サポートしているファイル形式か確認する
	/// </summary>
	/// <param name="filePath"></param>
	/// <returns></returns>
	static bool IsSupportedFile(const std::filesystem::path& filePath);

	/// <summary>
	/// ファイル一つに対してのmetaファイル作成
	/// </summary>
	/// <param name="srcFile"></param>
	/// <returns></returns>
	static nlohmann::json CreateMetaFileForFile(const std::filesystem::path& srcFile);


	static std::string AddressableName(const std::filesystem::path& srcFile);
};
}

