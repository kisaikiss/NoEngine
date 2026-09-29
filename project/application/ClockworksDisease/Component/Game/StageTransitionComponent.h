#pragma once
#include "engine/NoEngine.h"

// プレイヤーが触れたときに移動するシーンを指定する。
// 遷移させたいオブジェクトに付け、destinationScene に登録済みシーン名を設定する。
struct StageTransitionComponent {
	std::string destinationScene = "GameScene";
};
