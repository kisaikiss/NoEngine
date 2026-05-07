#include "application/TD3/Stage/MapTileRenderSystem.h"
#include "application/TD3/Utility/GameResourceComponent.h"
#include "application/TD3/Utility/SpriteLayer.h"

namespace Stage {

	void MapTileRenderSystem::Initialize(No::Entity managerEntity) {
		managerEntity_ = managerEntity;
		fieldDef_ = Stage::GetFieldLayerDef();
		bgFrontDef_ = Stage::GetBackgroundFrontLayerDef();
		bgBackDef_ = Stage::GetBackgroundBackLayerDef();
	}

	void MapTileRenderSystem::Update(No::Registry& registry, float) {
		auto* dirty = registry.GetComponent<Stage::MapRenderDirtyComponent>(managerEntity_);
		if (!dirty || !dirty->IsDirty()) return;

		auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
		if (!mapData) { dirty->Clear(); return; }

		if (dirty->fullReload) {
			ExecuteFullReload(registry, *mapData);
		} else {
			for (const auto& entry : dirty->dirtyTiles) {
				RefreshTileInternal(registry, *mapData, entry.layer, entry.x, entry.y);
			}
		}

		if (dirty->fieldAlphaDirty) SetLayerAlphaInternal(registry, TileLayer::Field, dirty->fieldAlpha);
		if (dirty->bgFrontAlphaDirty)  SetLayerAlphaInternal(registry, TileLayer::BackgroundFront, dirty->bgFrontAlpha);
		if (dirty->bgBackAlphaDirty)  SetLayerAlphaInternal(registry, TileLayer::BackgroundBack, dirty->bgBackAlpha);

		dirty->Clear();
	}

	void MapTileRenderSystem::ExecuteFullReload(No::Registry& registry, const MapData& mapData) {
		UnloadMap(registry);

		auto* res = registry.GetComponent<GameResourceComponent>(managerEntity_);
		NoEngine::TextureRef white = GetGameTextureOrWhite(*res, GameResourceKey::kWhiteTexture);

		int h = mapData.height;
		int w = mapData.width;

		fieldEntities_.assign(h, std::vector<No::Entity>(w));
		bgFrontEntities_.assign(h, std::vector<No::Entity>(w));
		bgBackEntities_.assign(h, std::vector<No::Entity>(w));

		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				// 奥から前: BackgroundBack → BackgroundFront → Field の順で生成
				bgBackEntities_[y][x] = CreateTileEntity(registry, white, x, y, ToLayer(SpriteLayer::BackgroundBackMapchip));
				bgFrontEntities_[y][x] = CreateTileEntity(registry, white, x, y, ToLayer(SpriteLayer::BackgroundFrontMapchip));
				fieldEntities_[y][x] = CreateTileEntity(registry, white, x, y, ToLayer(SpriteLayer::FieldMapChip));
			}
		}

		RefreshAllTiles(registry, mapData);
	}

	void MapTileRenderSystem::UnloadMap(No::Registry& registry) {
		auto destroyLayer = [&](std::vector<std::vector<No::Entity>>& entities) {
			for (auto& row : entities) {
				for (auto entity : row) { registry.DestroyEntity(entity); }
			}
			entities.clear();
			};
		destroyLayer(fieldEntities_);
		destroyLayer(bgFrontEntities_);
		destroyLayer(bgBackEntities_);
		registry.FlushDestroy();
	}

	void MapTileRenderSystem::RefreshTileInternal(No::Registry& registry, const MapData& mapData,
		TileLayer layer, int x, int y) {
		if (!mapData.InBounds(x, y)) return;

		const auto& tiles = GetTiles(mapData, layer);
		auto& entities = GetEntities(layer);
		const auto& layerDef = GetLayerDef(layer);

		if (y >= static_cast<int>(entities.size()) ||
			x >= static_cast<int>(entities[y].size())) return;

		auto* sprite = registry.GetComponent<No::SpriteComponent>(entities[y][x]);
		if (!sprite) return;

		const TileCell& cell = tiles[y][x];
		if (cell.IsEmpty()) {
			sprite->isVisible = false;
			return;
		}

		int idx = cell.textureIdx - 1;
		if (idx < 0 || idx >= static_cast<int>(layerDef.textureKeys.size())) {
			sprite->isVisible = false;
			return;
		}

		auto* res = registry.GetComponent<GameResourceComponent>(managerEntity_);
		if (!res) { sprite->isVisible = false; return; }

		NoEngine::TextureRef tex = GetGameTextureOrWhite(*res, layerDef.textureKeys[idx]);
		float texW = static_cast<float>(tex.GetWidth());
		float texH = static_cast<float>(tex.GetHeight());
		if (texW <= 0.f || texH <= 0.f) { sprite->isVisible = false; return; }

		float uvW = 64.f / texW;
		float uvH = 64.f / texH;
		sprite->textureHandle = tex;
		sprite->uv = { cell.tileX * uvW, cell.tileY * uvH, uvW, uvH };
		sprite->isVisible = true;
	}

	void MapTileRenderSystem::RefreshAllTiles(No::Registry& registry, const MapData& mapData) {
		for (int y = 0; y < mapData.height; y++) {
			for (int x = 0; x < mapData.width; x++) {
				RefreshTileInternal(registry, mapData, TileLayer::BackgroundBack, x, y);
				RefreshTileInternal(registry, mapData, TileLayer::BackgroundFront, x, y);
				RefreshTileInternal(registry, mapData, TileLayer::Field, x, y);
			}
		}
	}

	void MapTileRenderSystem::SetLayerAlphaInternal(No::Registry& registry, TileLayer layer, float alpha) {
		auto& entities = GetEntities(layer);
		for (auto& row : entities) {
			for (auto entity : row) {
				auto* sprite = registry.GetComponent<No::SpriteComponent>(entity);
				if (sprite) sprite->color.a = alpha;
			}
		}
	}

	No::Entity MapTileRenderSystem::CreateTileEntity(No::Registry& registry,
		NoEngine::TextureRef texture, int x, int y, uint32_t spriteLayer) {
		auto entity = registry.GenerateEntity();

		auto* t = registry.AddComponent<No::Transform2DComponent>(entity);
		t->translate = { static_cast<float>(x * TILE_SIZE), static_cast<float>(y * TILE_SIZE) };
		t->scale = { static_cast<float>(TILE_SIZE),     static_cast<float>(TILE_SIZE) };

		auto* s = registry.AddComponent<No::SpriteComponent>(entity);
		s->textureHandle = texture;
		s->layer = spriteLayer;
		s->pivot = { 0.f, 0.f };
		s->isVisible = false;
		s->uv = { 0.f, 0.f, 0.25f, 0.25f };

		return entity;
	}

	std::vector<std::vector<No::Entity>>& MapTileRenderSystem::GetEntities(TileLayer layer) {
		if (layer == TileLayer::BackgroundFront) return bgFrontEntities_;
		if (layer == TileLayer::BackgroundBack) return bgBackEntities_;
		return fieldEntities_;
	}

	const Stage::TileLayerDef& MapTileRenderSystem::GetLayerDef(TileLayer layer) const {
		if (layer == TileLayer::BackgroundFront) return bgFrontDef_;
		if (layer == TileLayer::BackgroundBack) return bgBackDef_;
		return fieldDef_;
	}

	const std::vector<std::vector<TileCell>>& MapTileRenderSystem::GetTiles(
		const MapData& mapData, TileLayer layer) const {
		if (layer == TileLayer::BackgroundFront) return mapData.bgTilesFront;
		if (layer == TileLayer::BackgroundBack) return mapData.bgTilesBack;
		return mapData.fieldTiles;
	}

} // namespace Stage
