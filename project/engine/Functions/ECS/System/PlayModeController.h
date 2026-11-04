#pragma once
#include "../Registry.h"
#include "externals/nlohmann/json.hpp"

namespace NoEngine {
namespace ECS {

/// <summary>
/// エディタの再生状態(Editing/Playing/Paused)とGameStopフラグ、
/// 再生開始時のシーンスナップショットを管理する。
/// </summary>
class PlayModeController {
public:
	enum class State {
		kEditing, // 未再生、または"■"で停止済み
		kPlaying, // ">"で再生中
		kPaused,  // 再生中に"||"で一時停止
	};

	static State GetState();

	/// <summary>Editing以外ならtrue。">"を押してから"■"を押すまでの間ずっとtrue</summary>
	static bool IsInPlayMode();

	/// <summary>trueの間、GetStopInGameStop()==trueのシステムは更新されない</summary>
	static bool IsGameStopped();

	/// <summary>">"を押した瞬間(Editing→Playing)に取得したシーンのスナップショット</summary>
	static const nlohmann::json& GetSnapshot();

	static void Play(Registry& registry);  // ">"
	static void Pause();                   // "||"
	static void Stop(Registry& registry);  // "■"

	/// <summary>SceneManagerが停止時のシーン復帰要求を一度だけ取り出します。</summary>
	static std::optional<std::string> TakeSceneRestoreRequest();

	/// <summary>元シーンの初期化後に、Registry全体のスナップショットを復元します。</summary>
	static void Update(Registry& registry);

private:
	PlayModeController() = delete;
};

}
}
