#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 敵共通の行動・デバッグ状態。
/// 主HPは HealthComponent に保持し、本コンポーネントは移動設定と接触フラグを保持する
/// </summary>
struct EnemyComponent {
	int hp = 10;
	int maxHp = 10;
	float moveSpeed = 3.0f;
	No::Vector3 moveDirection = { 0.0f, 0.0f, -1.0f };
	int groupId = 0;
	bool wasCollidingWithAttack = false;
	bool wasCollidingWithPlayer = false;
	int lastDamageTaken = 0;
};
