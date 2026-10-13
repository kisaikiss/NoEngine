#pragma once

namespace NoEngine {
namespace ECS {

/// <summary>
/// シーンJSON(resources/game/Scenes/&lt;scene&gt;.json)の "systems" 配列を読み書きする。
/// ファイルI/Oだけを担当し、システムの生成や適用はしない。
/// </summary>
namespace SystemSceneConfig {

/// <summary>シーンJSONのパス。シーン名が不正(空、/ \ : を含む)なら空文字</summary>
std::string GetScenePath(const std::string& sceneName);

/// <summary>
/// "systems" を読み込む。ファイルなし、形式不正、未登録のシステム名を含む場合は std::nullopt。
/// </summary>
std::optional<std::vector<std::string>> Load(const std::string& sceneName);

/// <summary>
/// "systems" を書き込む。JSONのその他のキー(entities等)は保持する。
/// </summary>
bool Save(const std::string& sceneName, const std::vector<std::string>& systemNames);

}
}
}