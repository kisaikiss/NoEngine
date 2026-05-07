#include "application/TD3/Stage/MapManager.h"
#include "application/TD3/Stage/MapDataIO.h"
#include "application/TD3/Stage/MapRenderDirtyComponent.h"
#include <algorithm>

namespace Stage {

	bool MapManager::LoadMap(No::Registry& registry, No::Entity managerEntity,
		const std::string& mapName, const std::string& basePath) {
		MapData loaded;
		if (!MapDataIO::Load(loaded, mapName, basePath)) return false;

		auto* mapData = registry.GetComponent<MapData>(managerEntity);
		if (!mapData) return false;
		*mapData = loaded;

		auto* dirty = registry.GetComponent<MapRenderDirtyComponent>(managerEntity);
		if (dirty) dirty->fullReload = true;
		return true;
	}

	bool MapManager::TransitionTo(No::Registry& registry, No::Entity managerEntity,
		const std::string& targetMapName,
		const std::string& targetPointId,
		No::Vector2& outSpawnPos) {
		if (!LoadMap(registry, managerEntity, targetMapName)) return false;

		auto* mapData = registry.GetComponent<MapData>(managerEntity);
		if (!mapData) return false;

		outSpawnPos = GetSpawnPos(*mapData, targetPointId);
		return true;
	}

	const TransitionPoint* MapManager::CheckTransition(
		const MapData& mapData, float playerPixelX, float playerPixelY) const {
		int gx = static_cast<int>(playerPixelX / TILE_SIZE);
		int gy = static_cast<int>(playerPixelY / TILE_SIZE);
		for (const auto& tp : mapData.transitions)
			if (tp.x == gx && tp.y == gy) return &tp;
		return nullptr;
	}

	No::Vector2 MapManager::GetMaxCameraOffset(const MapData& mapData) const {
		float maxX = std::max(0.f, mapData.width * static_cast<float>(TILE_SIZE) - 1280.f);
		float maxY = std::max(0.f, mapData.height * static_cast<float>(TILE_SIZE) - 720.f);
		return { maxX, maxY };
	}

	No::Vector2 MapManager::GetInitialSpawnPos(const MapData& mapData) const {
		return {
			static_cast<float>(mapData.initialSpawn.x * TILE_SIZE + TILE_SIZE / 2),
			static_cast<float>(mapData.initialSpawn.y * TILE_SIZE + TILE_SIZE / 2)
		};
	}

	No::Vector2 MapManager::GetSpawnPos(const MapData& map, const std::string& pointId) const {
		for (const auto& tp : map.transitions) {
			if (tp.id == pointId) {
				return {
					static_cast<float>(tp.x * TILE_SIZE + TILE_SIZE / 2),
					static_cast<float>(tp.y * TILE_SIZE + TILE_SIZE / 2)
				};
			}
		}
		return { 0.f, 0.f };
	}

} // namespace Stage
