#pragma once

namespace NoEngine {
namespace ECS {

/// <summary>
/// シーンJSON(resources/game/Scenes/&lt;scene&gt;.json)の "systems" 配列を保存する。
/// ファイルI/Oだけを担当し、システムの生成や適用はしない。
/// </summary>
namespace SystemSceneConfig {

/// <summary>シーンJSONのパス。シーン名が不正(空、/ \ : を含む)なら空文字</summary>
std::string GetScenePath(const std::string& sceneName);

/// <summary>
/// "systems" を書き込む。JSONのその他のキー(entities等)は保持する。
/// </summary>
bool Save(const std::string& sceneName, const std::vector<std::string>& systemNames);

}
}
}