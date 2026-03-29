#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 敵弾の運動・ダメージ情報。
/// 命中判定は EnemyBulletHitSystem で行い、ダメージ適用は DamageRequest 経由で行う
/// </summary>
struct EnemyBulletComponent {
	No::Vector3 velocity = { 0.0f, 0.0f, 0.0f };
	int damage = 1;
	No::Entity owner = No::nullEntity;
};
