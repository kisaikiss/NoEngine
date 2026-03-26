#pragma once
#include "engine/NoEngine.h"
#include <vector>

/// <summary>
/// プレイヤー攻撃のヒット情報
/// PlayerAttackResolveSystem が1回の攻撃で同一敵へ多重ヒットしないよう hitEnemies を管理する
/// </summary>
struct AttackDamageComponent {
	int damage = 10;
	std::vector<No::Entity> hitEnemies;
	bool isFirstFrameConsumed = false;
};
