#pragma once

// 移動方向
enum class Direction {
	None = 0,  // 停止中 or 初期状態
	Up,        // Y+ 方向
	Right,     // X+ 方向
	Down,      // Y- 方向
	Left       // X- 方向
};

// プレイヤーコンポーネント
// プレイヤーの位置、移動状態、パラメータを管理
struct PlayerComponent {
	// ========== 位置情報 ==========
	int currentNodeX;       // 現在いるノードのX座標
	int currentNodeY;       // 現在いるノードのY座標
	int targetNodeX;        // 移動先ノードのX座標（エッジ移動中のみ有効）
	int targetNodeY;        // 移動先ノードのY座標
	float progressOnEdge;   // エッジ上の進行度 [0.0 ~ 1.0]

	// ========== 移動状態 ==========
	Direction currentDirection;  // 現在の進行方向（None = 停止中）
	Direction lastDirection;     // 前回の進行方向（後退判定に使用）

	// ========== パラメータ ==========
	float moveSpeed;  // 移動速度（グリッド単位/秒）

	// ========== 状態フラグ ==========
	bool isOnEdge;     // エッジ移動中フラグ
	bool isOnNode;     // ノード上にいるフラグ
	bool isAtDeadEnd;  // 行き止まりフラグ

	PlayerComponent()
		: currentNodeX(1), currentNodeY(1),
		targetNodeX(1), targetNodeY(1),
		progressOnEdge(0.0f),
		currentDirection(Direction::None),
		lastDirection(Direction::None),
		moveSpeed(2.0f),
		isOnEdge(false),
		isOnNode(true),
		isAtDeadEnd(false) {
	}
};
