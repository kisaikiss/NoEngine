#pragma once
#include <string>

struct RailCameraComponent;

// ---------------------------------------------------------------------------
// パス生成
// ---------------------------------------------------------------------------
/// StageData/RailData/StageName_rail.json
std::string MakeRailFilePath(const std::string& stageName);
/// StageData/EventData/StageName_events.json
std::string MakeEventFilePath(const std::string& stageName);
/// StageData/StageName.json  （ラッパーJSON）
std::string MakeStageWrapperPath(const std::string& stageName);

// ---------------------------------------------------------------------------
// ランタイムリセット
// ---------------------------------------------------------------------------
void ResetEventRuntime(RailCameraComponent& rail);

// ---------------------------------------------------------------------------
// セーブ
// ---------------------------------------------------------------------------
bool SaveRailToJson(const RailCameraComponent& rail, const std::string& stageName);
bool SaveEventsToJson(const RailCameraComponent& rail, const std::string& stageName);
/// ステージラッパーJSONを保存する。SaveAll 時に呼ぶ。
bool SaveStageWrapper(const std::string& stageName);

// ---------------------------------------------------------------------------
// ロード
// ---------------------------------------------------------------------------
bool LoadRailToComponent(RailCameraComponent& rail, const std::string& stageName);
bool LoadEventsToComponent(RailCameraComponent& rail, const std::string& stageName);
/// ラッパーJSONの存在確認。ファイルがなくてもクラッシュしない（新ステージ作成用）。
bool LoadStageWrapper(const std::string& stageName);
