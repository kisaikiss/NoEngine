#pragma once
#include <string>
#include "StageData.h"

/// <summary>
/// マップローダー
/// ステージマニフェスト JSON を起点に、接続マップとエンティティマップを読み書きする。
///
/// 【ファイル構成】
///   resources/game/td_3105/Stages/stage_01.json          ← マニフェスト
///   resources/game/td_3105/Stages/stage_01_map.json      ← ノード接続情報
///   resources/game/td_3105/Stages/stage_01_entities.json ← エンティティ配置
///
/// 【Stage5 変更点】
///   LoadStage を追加。
///
/// 【Stage7 変更点】
///   SaveStage を追加。MapEditor から呼ばれる。
/// </summary>
class MapLoader {
public:
	/// <summary>
	/// ステージ全体を読み込んで StageData を返す。
	/// </summary>
	/// <param name="manifestPath">マニフェスト JSON のパス</param>
	static MapData::StageData LoadStage(const std::string& manifestPath);

	/// <summary>
	/// StageData を JSON ファイルに書き出す。
	/// ステージ番号からパスを自動生成する。
	/// マニフェスト・マップ・エンティティの 3 ファイルを書き出す。
	/// </summary>
	/// <param name="data">書き出すステージデータ</param>
	/// <param name="stageNumber">書き出し先のステージ番号（例: 1 → stage_01.json）</param>
	static void SaveStage(const MapData::StageData& data, int stageNumber);

	/// <summary>
	/// ステージ番号からマニフェストのパスを生成する。
	/// MapEditor のファイル存在チェックでも使用する。
	/// </summary>
	static std::string MakeManifestPath(int stageNumber);

private:
	static MapData::ConnectionMapData LoadConnectionMap(const std::string& path);
	static MapData::EntityMapData     LoadEntityMap(const std::string& path);

	/// <summary>
	/// ステージ番号から各ファイルのパスを生成する。
	/// 1  → "resources/game/td_3105/Stages/stage_01"
	/// 10 → "resources/game/td_3105/Stages/stage_10"
	/// </summary>
	static std::string MakeBasePath(int stageNumber);
};
