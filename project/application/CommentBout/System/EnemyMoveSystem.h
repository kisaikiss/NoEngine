#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 敵の移動のみを担当するSystem
/// </summary>
class EnemyMoveSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
