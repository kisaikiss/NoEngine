#pragma once
#include "externals/nlohmann/json.hpp"

namespace NoEngine {
class AssetManager {
public:
	// エディタ用データ構造
	struct EditorAssetData {
		std::string metaFilePath;          // metaファイルのパス
		std::string sourceFile;            // 元ファイル名
		std::string currentAddressableName;// 現在のAddressableName
		char nameInputBuffer[256];         // ImGuiで編集するための文字列バッファ
		bool isModified = false;           // 変更があったかどうか
	};

	/// <summary>
	/// resourcesフォルダ以下をクロールして、metaファイルを更新していく
	/// </summary>
	static void CreateMetaFileForAllFiles();

	/// <summary>
	/// 作成したmetaファイルを全削除(危険)
	/// </summary>
	static void DeleteAllMetaFiles();

	// metaファイルを探してApplicationで使用するリストを作成する
	static void CreateAddressableList();
	// metaファイルを読み込んでエディタ用リストを作る
	static void CreateEditorDataList();
	// 毎フレーム呼ぶImGui描画関数
	static void DrawImGui();

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


	static void SaveMetaFile(AssetManager::EditorAssetData& data);
};
}

