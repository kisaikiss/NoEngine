#pragma once

// プレイヤーの状態
enum class PlayerState {
	OnNode,				// ノード上で停止（progress = 0.0）
	MovingOnEdge,		// エッジ移動中
	StoppedOnEdge		// エッジ上で停止（progress ≠ 0.0）
};

// 移動方向
enum class Direction {
	None = 0,			// 停止中 or 初期状態
	Up,					// Y+ 方向
	Right,				// X+ 方向
	Down,				// Y- 方向
	Left				// X- 方向
};

/// <summary>
/// プレイヤーコンポーネント
///
/// 【座標設定について】
/// currentNodeX/Y の初期値はコンストラクタで (0,0) に設定されるが、
/// 実際の初期位置は SampleScene::InitializePlayer(registry, x, y) の引数で決定される。
/// コンストラクタの座標を直接変更しても反映されないので注意。
/// </summary>
struct PlayerComponent {

	/// 位置情報
	int currentNodeX;					// 現在いるノードのX座標
	int currentNodeY;					// 現在いるノードのY座標
	int targetNodeX;					// 移動先ノードのX座標（エッジ移動中のみ有効）
	int targetNodeY;					// 移動先ノードのY座標
	float progressOnEdge;				// エッジ上の進行度 [0.0 ~ 1.0]

	/// 移動状態
	PlayerState state;					// 現在の状態
	Direction currentDirection;			// 現在の進行方向（None = 停止中）
	Direction lastDirection;			// 前回の進行方向（後退判定に使用）
	Direction actualMovingDirection;	// 実際に移動していた方向（停止時の再開・モデル向きに使用）

	/// 入力履歴
	Direction recentInputs[4];			// 最近押された方向（最大4方向）
	int recentInputCount;				// 記録されている入力数
	float inputHistoryTime;				// 入力履歴の経過時間
	
	/// 終点近傍の先行入力
	// progress が NEAR_END_THRESHOLD 以上のとき、ターゲットノードで有効な方向のキーを押すと記録される。
	// OnReachNode で最優先に使用し、消費後は None に戻す。
	Direction bufferedDirection;

	/// パラメータ
	float moveSpeed;					// 移動速度（グリッド単位/秒）
	float inputHistoryWindow;			// 入力履歴の保持時間（秒）

	/// 状態フラグ
	bool isAtDeadEnd;					// 行き止まりフラグ

	/// プレイヤーが MovingOnEdge 状態のとき true。
	/// EnemyMovementSystem が参照して、
	/// プレイヤーが動いているときだけ敵を動かす。
	bool isMoving;

	/// 弾薬システム
	int currentBullets;					// 現在の弾数
	int maxBullets;						// 最大弾数
	float bulletSpeed;					// 弾の移動速度（ImGuiで調整可能）

	PlayerComponent()
		: currentNodeX(0), currentNodeY(0),		// 実際の初期位置は InitializePlayer の引数で設定
		targetNodeX(0), targetNodeY(0),
		progressOnEdge(0.0f),
		state(PlayerState::OnNode),
		currentDirection(Direction::None),
		lastDirection(Direction::None),
		actualMovingDirection(Direction::None),
		recentInputCount(0),
		inputHistoryTime(0.0f),
		bufferedDirection(Direction::None),
		moveSpeed(2.0f),
		inputHistoryWindow(0.15f),
		isAtDeadEnd(false),
		isMoving(false),
		currentBullets(5),
		maxBullets(10),
		bulletSpeed(3.0f)
	{
		for (int i = 0; i < 4; ++i) {
			recentInputs[i] = Direction::None;
		}
	}
};