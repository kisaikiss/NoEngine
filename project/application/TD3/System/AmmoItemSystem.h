#pragma once
#include "engine/NoEngine.h"
#include "../Component/AmmoItemComponent.h"
#include "../Component/PlayerComponent.h"

/// <summary>
/// 弾薬アイテムシステム
/// 弾薬アイテムの回収処理を管理する
/// </summary>
class AmmoItemSystem : public No::ISystem {
public:
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(No::Registry& registry, float deltaTime) override;
};
