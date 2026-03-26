#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// プレイヤー固有の操作設定。
/// PlayerControlSystem が参照し、Health/Invincibleとは分離して保持する
/// </summary>
struct PlayerComponent {
	float moveSpeed = 480.0f;
	float invincibleDurationDefault = 0.35f;
};
