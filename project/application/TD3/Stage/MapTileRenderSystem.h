#pragma once
#include "engine/NoEngine.h"
#include "application/TD3/Stage/MapData.h"
#include "application/TD3/Stage/MapLayerDef.h"
#include "application/TD3/Stage/MapRenderDirtyComponent.h"
#include <vector>

namespace Stage {

	/// <summary>
	/// MapData / MapRenderDirtyComponent をレジストリから読み取り、タイルエンティティを管理するシステム。
	///
	/// Update 内で MapRenderDirtyComponent を確認し:
	/// - fullReload: エンティティ再生成 (ExecuteFullReload)
	/// - dirtyTiles: 単タイル更新 (RefreshTileInternal)
	/// - fieldAlphaDirty / bgFrontAlphaDirty / bgBackAlphaDirty: 透明度変更 (SetLayerAlphaInternal)
	///
	/// 描画順 (奥→前): BackgroundBack → BackgroundFront → Field
	///
	/// MapEditorSystem / MapManager からは dirty flag 経由でのみ操作する。
	/// </summary>
	class MapTileRenderSystem : public No::ISystem {
	public:
		MapTileRenderSystem() { SetStopInGameStop(false); SetStopInPause(false); };

		void Initialize(No::Entity managerEntity);
		void Update(No::Registry& registry, float dt) override;

	private:
		No::Entity managerEntity_ = 0;
		Stage::TileLayerDef fieldDef_;
		Stage::TileLayerDef bgFrontDef_;
		Stage::TileLayerDef bgBackDef_;

		std::vector<std::vector<No::Entity>> fieldEntities_;
		std::vector<std::vector<No::Entity>> bgFrontEntities_;
		std::vector<std::vector<No::Entity>> bgBackEntities_;

		void ExecuteFullReload(No::Registry& registry, const MapData& mapData);
		void UnloadMap(No::Registry& registry);
		void RefreshTileInternal(No::Registry& registry, const MapData& mapData,
			TileLayer layer, int x, int y);
		void RefreshAllTiles(No::Registry& registry, const MapData& mapData);
		void SetLayerAlphaInternal(No::Registry& registry, TileLayer layer, float alpha);
		No::Entity CreateTileEntity(No::Registry& registry, NoEngine::TextureRef texture,
			int x, int y, uint32_t spriteLayer);
		std::vector<std::vector<No::Entity>>& GetEntities(TileLayer layer);
		const Stage::TileLayerDef& GetLayerDef(TileLayer layer) const;
		const std::vector<std::vector<TileCell>>& GetTiles(const MapData& mapData, TileLayer layer) const;
	};

} // namespace Stage
