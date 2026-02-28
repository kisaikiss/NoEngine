#pragma once
#include <string>
#include <vector>

/// <summary>
/// ステージデータ関連の構造体定義
/// MapLoader が JSON から読み込んだデータをこれらの構造体で返す
/// </summary>

namespace MapData {

	// ============================================================
	//  接続マップ
	// ============================================================

	/// <summary>
	/// ノード1つ分のデータ（JSON の "nodes" 配列の1要素に対応）
	/// </summary>
	struct NodeData {
		int  x, y;
		bool up, right, down, left;
		bool isEnemyOnly = false; // 敵専用道フラグ（プレイヤーは侵入不可）
	};

	/// <summary>
	/// 接続マップ全体（stage_XX_map.json に対応）
	/// </summary>
	struct ConnectionMapData {
		std::string	stageName;
		float gridScale;		// グリッド1マスのワールド単位（通常1.0f）
		std::vector<NodeData> nodes;
	};

	// ============================================================
	//  エンティティマップ
	// ============================================================

	/// <summary>
	/// エンティティ1つ分のデータ（JSON の "entities" 配列の1要素に対応）
	/// type: "player" / "enemy"
	/// enemyType: "normal" など（将来の敵種別拡張用）
	/// </summary>
	struct EntityData {
		std::string type;	// "player" or "enemy"
		int x, y;
		std::string enemyType; // 敵のみ使用（"normal" など）。player の場合は空文字。
	};

	/// <summary>
	/// エンティティマップ全体（stage_XX_entities.json に対応）
	/// </summary>
	struct EntityMapData {
		std::vector<EntityData> entities;
	};

	// ============================================================
	//  ステージデータ（全体）
	// ============================================================

	/// <summary>
	/// ステージ1ステージ分の全データ。
	/// MapLoader::LoadStage が返す。
	/// SampleScene::Setup でこれを受け取り、各 Initialize 関数に渡す。
	/// </summary>
	struct StageData {
		ConnectionMapData	connectionMap;
		EntityMapData	entityMap;
	};

} // namespace MapData
