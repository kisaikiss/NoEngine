#pragma once
#include "../Component/PlayerComponent.h" // PlayerState, Direction enum を流用

/// <summary>
/// 敵コンポーネント
/// PlayerComponent と同じ移動ロジックを使用するが、
/// 入力履歴・弾薬など「プレイヤー固有」のフィールドは持たない。
///
/// 移動方向の決定は EnemyMovementSystem が行う。
///   Stage3: グリーディーヒューリスティック（プレイヤー方向へ直進優先）
///   Stage4: BFS による最短経路探索（後退禁止制約考慮）
/// </summary>
struct EnemyComponent {

	/// 位置情報
	int currentNodeX;
	int currentNodeY;
	int targetNodeX;
	int targetNodeY;
	float progressOnEdge;

	/// 移動状態（PlayerState enum 流用）
	PlayerState state;
	Direction currentDirection;
	Direction lastDirection;
	Direction actualMovingDirection;

	/// 移動速度（グリッド単位/秒）
	float moveSpeed;

	EnemyComponent()
		: currentNodeX(0), currentNodeY(0),
		targetNodeX(0), targetNodeY(0),
		progressOnEdge(0.0f),
		state(PlayerState::OnNode),
		currentDirection(Direction::None),
		lastDirection(Direction::None),
		actualMovingDirection(Direction::None),
		moveSpeed(1.5f) {
	}
};