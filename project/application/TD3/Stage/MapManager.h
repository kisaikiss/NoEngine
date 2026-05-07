#pragma once
#include "engine/NoEngine.h"
#include "application/TD3/Stage/MapData.h"
#include <string>

namespace Stage {

	/// <summary>
	/// ランタイム時のマップ管理クラス。
	/// MapData コンポーネントに書き込み、MapRenderDirtyComponent に fullReload を立てる。
	/// MapTileRenderSystem のポインタは持たない。
	/// </summary>
	class MapManager {
	public:
		bool LoadMap(No::Registry& registry, No::Entity managerEntity,
			const std::string& mapName,
			const std::string& basePath = "resources/game/Stage");

		bool TransitionTo(No::Registry& registry, No::Entity managerEntity,
			const std::string& targetMapName,
			const std::string& targetPointId,
			No::Vector2& outSpawnPos);

		const TransitionPoint* CheckTransition(const MapData& mapData,
			float playerPixelX, float playerPixelY) const;

		No::Vector2 GetMaxCameraOffset(const MapData& mapData) const;

		/// <summary>
		/// StageXX_A の InitialSpawnPoint からワールド座標(px)を返す。
		/// LoadMap 後に呼ぶこと。
		/// </summary>
		No::Vector2 GetInitialSpawnPos(const MapData& mapData) const;

	private:
		No::Vector2 GetSpawnPos(const MapData& map, const std::string& pointId) const;
	};

} // namespace Stage
