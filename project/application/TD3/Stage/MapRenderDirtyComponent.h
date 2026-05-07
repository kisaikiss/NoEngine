#pragma once
#include "application/TD3/Stage/MapData.h"
#include <vector>

namespace Stage {

	struct MapRenderDirtyEntry {
		TileLayer layer;
		int x, y;
	};

	struct MapRenderDirtyComponent {
		bool fullReload = false;					// LoadMap 相当: エンティティ再生成
		std::vector<MapRenderDirtyEntry> dirtyTiles; // RefreshTile 相当: 単タイル更新

		bool  fieldAlphaDirty = false;
		float fieldAlpha = 1.f;
		bool  bgFrontAlphaDirty = false;
		float bgFrontAlpha = 1.f;
		bool  bgBackAlphaDirty = false;
		float bgBackAlpha = 1.f;

		void Clear() {
			fullReload = false;
			dirtyTiles.clear();
			fieldAlphaDirty = false;
			bgFrontAlphaDirty = false;
			bgBackAlphaDirty = false;
		}

		bool IsDirty() const {
			return fullReload || !dirtyTiles.empty()
				|| fieldAlphaDirty || bgFrontAlphaDirty || bgBackAlphaDirty;
		}
	};

} // namespace Stage
