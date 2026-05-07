#pragma once
#include "application/TD3/Stage/MapData.h"
#include <string>

namespace Stage {

	/// <summary>
	/// MapData の JSON 保存・読み込みを担う静的クラス。
	/// 保存先: resources/game/Stage/Stage{NN}/Stage{NN}_{X}.json
	/// 例: "Stage01_A" → resources/game/Stage/Stage01/Stage01_A.json
	/// </summary>
	class MapDataIO {
	public:
		/// <summary>マップデータをJSONとして保存する</summary>
		/// <param name="data">保存するマップデータ</param>
		/// <param name="basePath">保存先ベースパス</param>
		/// <returns>保存成功ならtrue</returns>
		static bool Save(const MapData& data,
			const std::string& basePath = "resources/game/Stage");

		/// <summary>JSONからマップデータを読み込む</summary>
		/// <param name="data">読み込み先</param>
		/// <param name="mapName">マップ名 ("Stage01_A" など)</param>
		/// <param name="basePath">読み込み元ベースパス</param>
		/// <returns>読み込み成功ならtrue</returns>
		static bool Load(MapData& data,
			const std::string& mapName,
			const std::string& basePath = "resources/game/Stage");

	private:
		// "Stage01_A" → "resources/game/Stage/Stage01/Stage01_A.json"
		static std::string BuildFilePath(const std::string& mapName,
			const std::string& basePath);
	};

} // namespace Stage
