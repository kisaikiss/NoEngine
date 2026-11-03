#pragma once

namespace NoEngine {
namespace Editor {

enum class EBrowserItemType {
	Model,
	Texture,
	Audio,
	Font,
	Prefab,
	Unknown
};

// ブラウザと各入力欄で同じ種類のペイロードを使う。
inline const char* GetAssetDragPayloadType(EBrowserItemType type) {
	switch (type) {
	case EBrowserItemType::Model: return "ASSET_MODEL_NAME";
	case EBrowserItemType::Texture: return "ASSET_TEXTURE_NAME";
	case EBrowserItemType::Audio: return "ASSET_AUDIO_NAME";
	case EBrowserItemType::Font: return "ASSET_FONT_NAME";
	case EBrowserItemType::Prefab: return "PREFAB_PATH";
	default: return "ASSET_PATH";
	}
}

// アセット/プレハブを問わず統一して扱うためのアイテム表現
struct BrowserItem {
	std::string displayName;          // グリッド表示用の名前
	std::string path;                 // 実体ファイルパス（Prefab: .json、Asset: ソースファイル）
	std::string metaFilePath;         // Assetのみ使用。Prefabの場合は空
	std::string sourceFilePath;       // プレビュー用の元ファイルパス
	std::string dragPayloadType;      // アセットの種類に応じたペイロード識別子
	std::string folderPath;			  // resources/ からの相対パス（"" ならルート直下）
	EBrowserItemType type = EBrowserItemType::Unknown;

	// インスペクタ編集用（Assetのみ使用）
	char nameInputBuffer[128] = {};
	std::string currentAddressableName;
	bool isModified = false;
};

struct FolderNode {
	std::string name;         // 表示名（フォルダ名のみ）
	std::string relativePath; // resources/ からの相対パス。ルートは ""
	std::vector<FolderNode> children;
};

} // namespace Editor
} // namespace NoEngine
