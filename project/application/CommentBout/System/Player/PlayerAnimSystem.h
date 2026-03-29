#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// プレイヤーのスプライトアニメーション切り替えと無敵中の透明点滅を担当する。
/// DamageFlashSystem の後・SpriteAnimationSystem の前に実行する。
/// </summary>
class PlayerAnimSystem : public No::ISystem {
public:
    void Update(No::Registry& registry, float deltaTime) override;
};
