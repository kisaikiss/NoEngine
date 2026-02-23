#pragma once
#include "engine/NoEngine.h"
#include "../Component/PlayerBulletComponent.h"

/// <summary>
/// プレイヤー弾丸システム
/// 弾丸の移動と消滅を管理する
/// </summary>
class PlayerBulletSystem : public No::ISystem {
public:
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(No::Registry& registry, float deltaTime) override;
};
