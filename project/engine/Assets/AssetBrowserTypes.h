#pragma once

namespace NoEngine {
namespace Editor {

enum class EBrowserItemType {
	Model,
	Texture,
	Audio,
	Prefab,
	Unknown
};

// アセット/プレハブを問わず統一して扱うためのアイテム表現
struct BrowserItem {
	std::string displayName;          // グリッド表示用の名前
	std::string path;                 // 実体ファイルパス（Prefab: .json、Asset: ソースファイル）
	std::string metaFilePath;         // Assetのみ使用。Prefabの場合は空
	std::string dragPayloadType;      // "PREFAB_PATH" or "ASSET_PATH"
	EBrowserItemType type = EBrowserItemType::Unknown;

	// インスペクタ編集用（Assetのみ使用）
	char nameInputBuffer[128] = {};
	std::string currentAddressableName;
	bool isModified = false;
};

} // namespace Editor
} // namespace NoEngine