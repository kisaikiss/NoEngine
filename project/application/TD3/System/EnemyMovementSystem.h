#pragma once
#include "engine/NoEngine.h"
#include "../Component/EnemyComponent.h"
#include "../Component/PlayerComponent.h"

// 前方宣言
class GameTimer;

/// <summary>
/// 敵移動システム
/// player->isMoving が true のときのみ敵を動かす。
/// ChooseDirection を BFS 最短経路探索に差し替えた。
/// 後退禁止制約は最初の1歩のみ適用し、
/// 2歩目以降は制約なしで探索することで迂回ルートにも対応する。
/// 
/// ゲームタイマーを使用して、プレイヤーが移動中のみ敵が動く仕様を実現する。
/// </summary>
class EnemyMovementSystem : public No::ISystem {
public:
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(No::Registry& registry, float deltaTime) override;

	/// <summary>
	/// ゲームタイマーを設定
	/// </summary>
	void SetGameTimer(GameTimer* timer) { gameTimer_ = timer; }

private:
	GameTimer* gameTimer_ = nullptr;

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
	/// 次に移動する方向を BFS で決定する（Stage4）
	///
	/// 敵の currentNode からプレイヤーの currentNode への最短経路を BFS で探索し、
	/// 「最初の1歩の方向」だけを返す。次のノード到達時に再計算するため1ステップで十分。
	///
	/// 後退禁止制約：
	///   最初の1歩のみ GridUtils::CanMoveInDirection(... lastDirection) を適用。
	///   2歩目以降は GridUtils::CanMoveInDirection(... Direction::None) で制約なし。
	///
	/// 到達不能な場合は Direction::None を返す（敵は停止）。
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