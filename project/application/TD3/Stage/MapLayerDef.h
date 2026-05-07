#pragma once
#include <string>
#include <vector>

namespace Stage {

	/// <summary>
	/// タイルレイヤーの定義構造体
	/// </summary>
	struct TileLayerDef {
		std::string layerName;
		std::vector<std::string> textureKeys;
	};

	/// <summary>
	/// フィールドのタイル画像定義を返す
	/// GameResourceComponent の textureKeys と対応させる!
	/// </summary>
	inline const TileLayerDef& GetFieldLayerDef() {
		static TileLayerDef def{
			"Field",
			{ "FieldBlock01", "FieldBlock02" }
		};
		return def;
	}

	/// <summary>
	/// 背景前レイヤーのタイル画像定義を返す (フィールドの直後、BackgroundBack の手前)
	/// GameResourceComponent の textureKeys と対応させる!
	/// </summary>
	inline const TileLayerDef& GetBackgroundFrontLayerDef() {
		static TileLayerDef def{
			"BackgroundFront",
			{ "BackgroundBlock01" }
		};
		return def;
	}

	/// <summary>
	/// 背景後ろレイヤーのタイル画像定義を返す (最奥)
	/// GameResourceComponent の textureKeys と対応させる!
	/// </summary>
	inline const TileLayerDef& GetBackgroundBackLayerDef() {
		static TileLayerDef def{
			"BackgroundBack",
			{ "BackgroundBlock01", "BackgroundBlock02" }
		};
		return def;
	}

} // namespace Stage
