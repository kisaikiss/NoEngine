#pragma once
#include "engine/NoEngine.h"

// プレイヤーが触れたときに移動するシーンを指定する。
// 遷移させたいオブジェクトに付け、destinationScene に登録済みシーン名を設定する。
struct StageTransitionComponent {
	std::string destinationScene = "GameScene";
	float transitionTime = 1.0f;
	No::Vector3 returnPosition = No::Vector3::ZERO;

	float scalingTimer = 0.0f;
	No::Vector3 collidePosition = No::Vector3::ZERO;
};