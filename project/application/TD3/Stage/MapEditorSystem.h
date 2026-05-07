#pragma once
#ifdef USE_IMGUI
#include "engine/NoEngine.h"
#include "application/TD3/Stage/MapData.h"
#include <string>
#include <unordered_map>

struct GameStateComponent;
namespace Stage { class MapManager; }

/// <summary>
/// マップエディタシステム。
/// editorMode=true のとき ImGui UI でタイル配置・マップ設定・保存/読み込みを行う。
/// TAB で editorMode をトグルする。
/// MapData / MapRenderDirtyComponent はレジストリ経由でのみアクセスする。
/// </summary>
class MapEditorSystem : public No::ISystem {
public:
	MapEditorSystem() { SetStopInGameStop(false); SetStopInPause(false); };

	/// <summary>
	/// GameManagerエンティティと Camera2D エンティティを設定する。
	/// AddSystem 後、エンティティ生成後に呼ぶこと。
	/// </summary>
	void Initialize(No::Entity managerEntity, No::Entity cam2dEntity,
		No::Entity playerEntity, Stage::MapManager* mapManager = nullptr);

	void Update(No::Registry& registry, float dt) override;

private:
	No::Entity managerEntity_ = 0;
	No::Entity cam2dEntity_ = 0;
	No::Entity playerEntity_ = 0;
	bool initialized_ = false;
	Stage::MapManager* mapMgr_ = nullptr;

	// --- エディタ状態 ---
	enum class EditorMode { Field, BackgroundFront, BackgroundBack, Object };
	EditorMode mode_ = EditorMode::Field;

	// --- ゲームカメラ（エディタでのプレビュー用） ---
	float gameCamX_ = 0.f;
	float gameCamY_ = 0.f;
	float gameCamStep_ = 40.f;

	// --- カメラパン ---
	float camOffsetX_ = 0.f;
	float camOffsetY_ = 0.f;
	float panStartX_ = 0.f;
	float panStartY_ = 0.f;
	float panCamStartX_ = 0.f;
	float panCamStartY_ = 0.f;
	bool  isPanning_ = false;

	// --- エディタキャンバス矩形（スクリーン座標） ---
	float canvasPosX_ = 0.f;
	float canvasPosY_ = 0.f;
	float canvasWidth_ = 0.f;
	float canvasHeight_ = 0.f;
	bool  hasCanvasRect_ = false;
	bool  isCanvasHovered_ = false;
	bool  isCanvasDragging_ = false;

	// --- ImGui描画用テクスチャIDキャッシュ ---
	std::unordered_map<std::string, ImTextureID> texIdCache_;

	// --- タイルパレット選択状態 ---
	int selectedTexIdx_ = 1;
	int selectedTileX_ = 0;
	int selectedTileY_ = 0;

	// --- Object モード ---
	int selectedObjType_ = 0;
	int placingTpIdx_ = 0;

	// --- 表示スケール ---
	// 32x18 タイルがキャンバスに収まるよう自動計算される（縮小のみ）
	float tileDisplaySize_ = static_cast<float>(Stage::TILE_SIZE);

	// --- マップサイズ変更用バッファ ---
	int pendingWidth_ = 32;
	int pendingHeight_ = 18;

	// --- 未保存状態管理 ---
	bool isDirty_ = false;

	enum class PendingAction { None, SwitchToPlay };
	PendingAction pendingAction_ = PendingAction::None;
	bool          showUnsavedDialog_ = false;

	// --- アンドゥ/リドゥ ---
	struct TileEditCommand {
		Stage::TileLayer layer;
		int x, y;
		Stage::TileCell before;
		Stage::TileCell after;
	};

	struct ObjectSnapshot {
		Stage::InitialSpawnPoint         initialSpawn;
		std::vector<Stage::RespawnPoint> respawnPoints;
		std::vector<Stage::TransitionPoint> transitions;
	};

	std::vector<TileEditCommand> tileUndoStack_;
	std::vector<TileEditCommand> tileRedoStack_;
	std::vector<ObjectSnapshot>  objUndoStack_;
	std::vector<ObjectSnapshot>  objRedoStack_;
	static constexpr int kMaxUndoHistory = 100;

	// --- 内部メソッド ---
	void InitializeDefaultMap(No::Registry& registry);
	void HandleEditorToggle(No::Registry& registry);
	void DoEditorToggle(No::Registry& registry, GameStateComponent* state);
	void HandleMouseInput(No::Registry& registry);
	void DrawEditorWindow(No::Registry& registry);
	void DrawUnsavedDialog(No::Registry& registry);
	void ExecutePendingAction(No::Registry& registry);
	void DrawSection_File(No::Registry& registry);
	void DrawSection_MapSettings(No::Registry& registry);
	void DrawSection_Layer(No::Registry& registry);
	void DrawSection_Camera(No::Registry& registry);
	void DrawTilePalette(No::Registry& registry);
	void DrawObjectPalette(No::Registry& registry);
	void DrawMapTiles(No::Registry& registry, ImDrawList* dl);
	void DrawTile(ImDrawList* dl, ImTextureID tex, int gx, int gy, ImU32 tint,
		const Stage::TileCell& cell, float texW, float texH) const;
	void DrawGridOverlay(ImDrawList* dl, const Stage::MapData& mapData) const;
	void DrawObjectOverlay(ImDrawList* dl, const Stage::MapData& mapData) const;
	void ClampCameraToMap(const Stage::MapData& mapData);
	void ApplyLayerAlpha(No::Registry& registry);
	ImTextureID GetCachedImTextureID(const std::string& key, const NoEngine::TextureRef& tex);
	void PlaceTile(No::Registry& registry, Stage::TileLayer layer, int x, int y);
	void EraseTile(No::Registry& registry, Stage::TileLayer layer, int x, int y);
	void PlaceObject(No::Registry& registry, int x, int y);
	void EraseObject(No::Registry& registry, int x, int y);
	void PushTileUndo(const TileEditCommand& cmd);
	void PushObjectUndo(const Stage::MapData& before);
	void HandleUndoRedo(No::Registry& registry);
	void DoSave(No::Registry& registry);
	void DoLoad(No::Registry& registry);
	std::pair<int, int> ScreenToGrid(float screenX, float screenY) const;
	No::Vector2 GridToScreen(int gx, int gy) const;
};

#endif // USE_IMGUI
