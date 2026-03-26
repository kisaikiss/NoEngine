#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// プレイヤー固有の操作設定。
/// PlayerControlSystem が参照し、Health/Invincibleとは分離して保持する
/// </summary>
struct PlayerComponent {
	float moveSpeed = 480.0f;
	float acceleration = 2400.0f;
	float deceleration = 3000.0f;
	float maxSpeed = 480.0f;
	No::Vector2 velocity = { 0.0f, 0.0f };
	bool debugInvincible = false;			/// デバッグ用無敵フラグ
	float invincibleDurationDefault = 0.35f;
};
