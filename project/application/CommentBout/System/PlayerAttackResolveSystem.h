#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 自機攻撃の命中解決を担当するSystem
/// 攻撃の当たり判定を行い、HealthComponentへのダメージ適用要求を発行する
/// スクリーン投影 + サンプル遮蔽判定で可視部分のみヒットを成立させる
/// </summary>
class PlayerAttackResolveSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
