#pragma once
#include "../Component/PlayerComponent.h" // PlayerState, Direction enum を流用

/// <summary>
/// 敵コンポーネント
/// PlayerComponent と同じ移動ロジックを使用するが、
/// 入力履歴・弾薬など「プレイヤー固有」のフィールドは持たない。
///
/// 移動方向の決定は EnemyMovementSystem が行う。
/// BFS による最短経路探索（後退禁止）
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

	///敵同士の衝突を反転させるためのクールダウンタイマ
	float reverseTimer;
	static constexpr float REVERSE_COOLDOWN = 1.0f; // 反転後に衝突を無視する時間（秒）

	// ========== スポーニング状態 ==========
	// EnemySpawnerから生成された直後のモード
	// isSpawning=true の間、自機・自機弾・衝撃波・敵同士の衝突判定を受けない。

	bool  isSpawning = false;			// スポーニング状態フラグ
	float spawningSpeed = 0.0f;			// 敵専用道上の移動速度（EnemySpawnerSystemが計算して設定）
	float spawnExitTimer = 0.0f;		// 通常ノード到達後のカウントダウン（GameTimer基準）
	static constexpr float SPAWN_EXIT_DURATION = 0.5f; // 通常ノード到達後、何秒で通常状態に移行するか

	// ========== 色情報 ==========
	No::Color spawningColor;			// スポーニング状態の色（白）
	No::Color defaultColor;				// 通常状態の色（赤）

	EnemyComponent()
		: currentNodeX(0), currentNodeY(0),
		targetNodeX(0), targetNodeY(0),
		progressOnEdge(0.0f),
		state(PlayerState::OnNode),
		currentDirection(Direction::None),
		lastDirection(Direction::None),
		actualMovingDirection(Direction::None),
		moveSpeed(1.5f),
		reverseTimer(0.0f),
		isSpawning(false),
		spawningSpeed(0.0f),
		spawnExitTimer(0.0f),
		spawningColor({ 1.0f, 1.0f, 1.0f, 1.0f }),		// 白
		defaultColor({ 1.0f, 0.2f, 0.2f, 1.0f })		// 赤
	{
	}
};