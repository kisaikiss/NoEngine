#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 敵撃破時に生成される報酬オーブの移動状態。
/// EnemyRewardToBossSystem が二次ベジェで移動し、到達時にボスへダメージを送る
/// </summary>
struct EnemyRewardOrbComponent {
	No::Vector2 start = { 0.0f, 0.0f };
	No::Vector2 control = { 0.0f, 0.0f };
	No::Vector2 end = { 0.0f, 0.0f };
	float duration = 0.8f;
	float elapsed = 0.0f;
	int attackPower = 1;
};
