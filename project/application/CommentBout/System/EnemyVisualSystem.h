#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 敵の見た目更新とデバッグ描画を担当するSystem。
/// </summary>
class EnemyVisualSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
