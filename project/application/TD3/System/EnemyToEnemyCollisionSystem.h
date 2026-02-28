#pragma once
#include "engine/NoEngine.h"
#include "../Component/EnemyComponent.h"

/// <summary>
/// 敵同士衝突システム
///
/// CollisionSystem が設定した isCollied フラグを参照し、 衝突相手が kEnemy だったとき両者を反転させる。
/// 
/// 【振動防止】
///   反転直後に reverseTimer = REVERSE_COOLDOWN をセットし、タイマーが残っている間は衝突を無視する。
///   タイマーの減算は EnemyMovementSystem::Update 内で行う。
/// </summary>
class EnemyToEnemyCollisionSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;

private:
	/// <summary>
	/// 敵1体を反転させる。
	/// currentNode ⇔ targetNode を入れ替え、progress を鏡像にする。
	/// </summary>
	void ReverseEnemy(EnemyComponent* enemy);
};