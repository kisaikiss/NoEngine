#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 敵とプレイヤーの接触ダメージ解決を担当するSystem。
/// 自機接触は「スプライト重なり + カメラゲート」の共通判定で処理する。
/// </summary>
class EnemyContactDamageSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
