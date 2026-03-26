#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 敵弾の命中判定とダメージ処理を行うシステム。
/// </summary>
class EnemyBulletHitSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
