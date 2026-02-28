#pragma once
#include "../Component/PlayerComponent.h" // Direction enum

/// <summary>
/// 敵スポナーコンポーネント
/// EnemySpawnerTag を持つエンティティに付与される。
/// 自身が置かれたノードから spawnDirection に向かって敵を生成する。
/// </summary>
struct EnemySpawnerComponent {

	int nodeX = 0;				// 配置グリッド座標X
	int nodeY = 0;				// 配置グリッド座標Y

	Direction spawnDirection = Direction::None; // 敵を送り出す方向（SetupSpawnersで自動検出）

	int   chainCount       = 0;    // 敵専用道の連なり数（起動時に計算）
	float calculatedSpeed  = 1.0f; // chainCount / spawnInterval（起動時に計算）

	float spawnTimer = 0.0f;       // スポーンまでの現在のカウンタ（GameTimer基準）
};
