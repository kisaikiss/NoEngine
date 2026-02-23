#pragma once
#include "engine/NoEngine.h"
#include "../Component/EnemyComponent.h"
#include "../Component/PlayerComponent.h"

/// <summary>
/// 敵移動システム
/// player->isMoving が true のときのみ敵を動かす。
/// Stage3: グリーディーヒューリスティック（プレイヤー方向優先）
/// Stage4: BFS 最短経路探索に差し替える予定
/// </summary>
class EnemyMovementSystem : public No::ISystem {
public:
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(No::Registry& registry, float deltaTime) override;
private:
	// ========== 状態別の移動処理 ==========

	/// <summary>
	/// ノード上にいるときの処理
	/// </summary>
	void HandleOnNode(EnemyComponent* enemy, int playerX, int playerY, No::Registry& registry);
	
	/// <summary>
	/// エッジ移動中の処理
	/// </summary>
	void HandleOnEdge(EnemyComponent* enemy, float deltaTime, int playerX, int playerY, No::Registry& registry);
	
	/// <summary>
	/// ノード到達時の処理
	/// </summary>
	void OnReachNode(EnemyComponent* enemy, int playerX, int playerY, No::Registry& registry);
	/// <summary>
	/// 指定方向への移動を開始する
	/// </summary>
	void StartMovement(EnemyComponent* enemy, Direction dir);

	// ========== 経路選択 ==========

	/// <summary>
	/// 次に移動する方向を決定する（Stage3: グリーディーヒューリスティック）
	/// プレイヤーへのマンハッタン距離を最も減らす方向を優先的に試みる。
	/// GridUtils::CanMoveInDirection で後退禁止 + 行き止まり例外（A案）を適用。
	/// Stage4 でこの関数を BFS に差し替える予定。
	/// </summary>
	Direction ChooseDirection(EnemyComponent* enemy, int playerX, int playerY, No::Registry& registry);

	// ========== Transform 更新 ==========

	/// <summary>
	/// 敵のワールド座標を計算する
	/// OnNode のときはノード座標、エッジ上は線形補間で求める
	/// </summary>
	No::Vector3 CalculateWorldPosition(const EnemyComponent* enemy);

	/// <summary>
	/// 位置更新
	/// </summary>
	void UpdateTransform(const EnemyComponent* enemy, No::TransformComponent* transform);

#ifdef USE_IMGUI

	/// <summary>
	/// 全敵の状態を表示するデバッグウィンドウ
	/// </summary>
	void DebugUI(No::Registry& registry);
	const char* DirectionToString(Direction dir);

#endif
};