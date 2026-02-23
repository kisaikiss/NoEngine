#pragma once
#include "engine/NoEngine.h"
#include "../Component/PlayerBulletComponent.h"

/// <summary>
/// プレイヤーの弾システム
/// 弾の移動、敵との衝突、壁判定を行う。
/// 
/// 【処理の流れ】
/// 1. 移動処理（direction × speed × deltaTime）
/// 2. 敵との衝突判定（CollisionSystemの結果を参照）
/// 3. 壁判定（グリッドベース）
/// </summary>
class PlayerBulletSystem : public No::ISystem {
public:
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(No::Registry& registry, float deltaTime) override;

private:
	/// <summary>
	/// 指定ノードで弾が消滅すべきか判定する（壁判定）
	/// </summary>
	bool ShouldDestroyAtNode(
		No::Registry& registry,
		int nodeX, int nodeY,
		const No::Vector3& direction
	);
};