#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 敵の撃破報酬オーブをボスへ飛ばすシステム
/// </summary>
class EnemyRewardToBossSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
