#pragma once
#include <string>
#include "StageData.h"

/// <summary>
/// マップローダー
/// ステージマニフェスト JSON を起点に、接続マップとエンティティマップを読み込む。
///
/// 【ファイル構成】
///   resources/stages/stage_01.json          ← マニフェスト（このパスを LoadStage に渡す）
///   resources/stages/stage_01_map.json      ← ノード接続情報
///   resources/stages/stage_01_entities.json ← エンティティ配置
///
/// 【使用例】
///   StageData data = MapLoader::LoadStage("resources/stages/stage_01.json");
/// </summary>
class MapLoader {
public:
	/// <summary>
	/// ステージ全体を読み込んで StageData を返す。
	/// 内部でマニフェストを解析し、接続マップとエンティティマップを読み込む。
	/// </summary>
	/// <param name="manifestPath">マニフェスト JSON のパス</param>
	/// <returns>読み込んだステージデータ</returns>
	static MapData::StageData LoadStage(const std::string& manifestPath);

private:
	/// <summary>
	/// 接続マップ JSON（stage_XX_map.json）を読み込む
	/// </summary>
	static MapData::ConnectionMapData LoadConnectionMap(const std::string& path);

	/// <summary>
	/// エンティティマップ JSON（stage_XX_entities.json）を読み込む
	/// </summary>
	static MapData::EntityMapData LoadEntityMap(const std::string& path);
};
