#pragma once
#include <string>

struct RailCameraComponent;
///レールカメらの制御点やイベントのデータをJSONファイルファイルパス
std::string MakeRailFilePath(const std::string& stageName);
///ゲームイベントをJSONファイルファイルパス
std::string MakeEventFilePath(const std::string& stageName);

/// <summary>
/// レールイベントの実行状態をリセットします。
/// </summary>
/// <param name="rail"></param>
void ResetEventRuntime(RailCameraComponent& rail);

///情報のセーブ
bool SaveRailToJson(const RailCameraComponent& rail, const std::string& stageName);
bool SaveEventsToJson(const RailCameraComponent& rail, const std::string& stageName);

///情報のロード
bool LoadRailToComponent(RailCameraComponent& rail, const std::string& stageName);
bool LoadEventsToComponent(RailCameraComponent& rail, const std::string& stageName);
