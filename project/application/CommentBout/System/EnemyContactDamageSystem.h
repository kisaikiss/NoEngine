#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 敵とプレイヤーの接触ダメージ解決を担当するSystem。
/// </summary>
class EnemyContactDamageSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
