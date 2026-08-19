#pragma once
#include <string>

namespace NoEngine {
namespace Event {

// 組み込みの遷移演出の種類。
// kCustom を指定した場合は SceneManager::RegisterTransitionEffect で登録した
// customTransitionName の演出が使われる。
enum class SceneTransitionType {
	kImmediate,   // 演出なし即時切り替え
	kCircleScale, // 既存の円形マスク演出（デフォルト）
	kFade,        // 単色フェード演出
	kCustom,      // アプリケーション側で自由に追加した演出
};

struct SceneChangeEvent {
	std::string nextScene;
	SceneTransitionType transitionType = SceneTransitionType::kCircleScale;
	// transitionType == kCustom の時だけ参照される、登録済みの演出名
	std::string customTransitionName;
};

}
}