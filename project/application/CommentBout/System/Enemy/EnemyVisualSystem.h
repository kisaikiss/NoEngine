#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 敵の見た目更新と敵状態デバッグ表示を担当するSystem。
/// 衝突デバッグ描画は CollisionDebugRenderSystem に分離する。
/// </summary>
class EnemyVisualSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
