#pragma once
#include <string>
#include <vector>

namespace Stage {

	enum class TileLayer { Field, BackgroundFront, BackgroundBack };

	/// <summary>
	/// オブジェクト配置情報（敵・アイテムなど）
	/// </summary>
	struct ObjectPlacement {
		std::string type;	// "enemy_slime" など
		int x = 0;
		int y = 0;
	};

	/// <summary>
	/// 初期スポーン地点。1マップに1つ。
	/// ゲーム開始時およびマップロード直後のプレイヤー配置座標になる。
	/// </summary>
	struct InitialSpawnPoint {
		int x = 0;
		int y = 0;
	};

	/// <summary>
	/// 中間リスポーン地点。複数配置可能。
	/// プレイヤーが踏んだ時点で有効化され、以降の死亡時にここへ戻る。
	/// </summary>
	struct RespawnPoint {
		std::string id;		// このマップ内での識別子 ("01", "02", ...)
		int x = 0;
		int y = 0;
	};

	/// <summary>
	/// マップ間移動ポイント。
	/// id で識別し、targetMap + targetPoint で行き先を指定する。
	/// 同一マップに複数配置可能で、それぞれ独立した行き先を持てる。
	/// </summary>
	struct TransitionPoint {
		std::string id;			// このマップ内での識別子 ("01", "02", ...)
		int x = 0;				// グリッド座標
		int y = 0;
		std::string targetMap;	// 行き先マップ名 ("Stage01_B")
		std::string targetPoint;// 行き先ポイントid ("02")
	};

	struct TileCell {
		int textureIdx = 0;
		int tileX = 0;
		int tileY = 0;

		bool IsEmpty() const { return textureIdx == 0; }

		bool operator==(const TileCell& other) const {
			return textureIdx == other.textureIdx
				&& tileX == other.tileX
				&& tileY == other.tileY;
		}
	};

	/// <summary>
	/// 1マップ分のデータ。
	/// ファイル名は GetMapName() で生成する ("Stage01_A" など)。
	/// </summary>
	struct MapData {
		int  stageNo = 1;
		char mapId = 'A';
		int  width = 32;	// デフォルトは1画面分
		int  height = 18;	// デフォルトは1画面分

		std::vector<std::vector<TileCell>> fieldTiles;
		std::vector<std::vector<TileCell>> bgTilesFront;  // 背景前レイヤー (フィールド直後)
		std::vector<std::vector<TileCell>> bgTilesBack;  // 背景後ろレイヤー (最奥)

		InitialSpawnPoint initialSpawn;
		std::vector<RespawnPoint> respawnPoints;
		std::vector<ObjectPlacement> objects;
		std::vector<TransitionPoint> transitions;

		/// <summary>ファイル名を生成する。例: "Stage01_A"</summary>
		std::string GetMapName() const {
			std::string s = "Stage";
			s += (stageNo < 10 ? "0" : "");
			s += std::to_string(stageNo);
			s += "_";
			s += mapId;
			return s;
		}

		/// <summary>タイル配列を width×height で初期化（全て空）</summary>
		void Initialize() {
			TileCell empty{};
			fieldTiles.assign(height, std::vector<TileCell>(width, empty));
			bgTilesFront.assign(height, std::vector<TileCell>(width, empty));
			bgTilesBack.assign(height, std::vector<TileCell>(width, empty));
		}

		void Resize(int newW, int newH) {
			TileCell empty{};
			fieldTiles.resize(newH, std::vector<TileCell>(width, empty));
			bgTilesFront.resize(newH, std::vector<TileCell>(width, empty));
			bgTilesBack.resize(newH, std::vector<TileCell>(width, empty));
			for (auto& row : fieldTiles) { row.resize(newW, empty); }
			for (auto& row : bgTilesFront) { row.resize(newW, empty); }
			for (auto& row : bgTilesBack) { row.resize(newW, empty); }
			width = newW;
			height = newH;
		}

		/// <summary>グリッド座標が範囲内かチェック</summary>
		bool InBounds(int x, int y) const {
			return x >= 0 && x < width && y >= 0 && y < height;
		}
	};

	// タイル定数
	constexpr int TILE_SIZE = 40;		// 1タイルの表示サイズ (px)
	constexpr int TILESET_COLS = 4;		// タイルセット列数
	constexpr int TILESET_ROWS = 4;		// タイルセット行数
	// 1画面 = 32x18 タイル (1280/40=32, 720/40=18)
	// 最大マップ = 画面2x2枚 = 64x36 タイル
	constexpr int MAX_MAP_WIDTH = 64;	// 最大マップ幅 (タイル数)
	constexpr int MAX_MAP_HEIGHT = 36;	// 最大マップ高さ (タイル数)

} // namespace Stage
