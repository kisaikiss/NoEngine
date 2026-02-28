#pragma once
#include "engine/NoEngine.h"
#include "../Component/EnemySpawnerComponent.h"

// 前方宣言
class GameTimer;

/// <summary>
/// 敵スポナーシステム
///	ステージロード後に SampleScene から呼ぶ。
///	各スポナーの spawnDirection を自動検出し、chainCount と calculatedSpeed を計算する。
/// </summary>
class EnemySpawnerSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;

	/// <summary>
	/// ゲームタイマーを設定
	/// </summary>
	void SetGameTimer(GameTimer* timer) { gameTimer_ = timer; }

	/// <summary>
	/// ステージロード後に呼ぶ初期化処理。
	/// 各スポナーの spawnDirection・chainCount・calculatedSpeed を計算する。
	/// ReloadStage のたびに再呼び出しが必要。
	/// </summary>
	void SetupSpawners(No::Registry& registry);

private:
	GameTimer* gameTimer_ = nullptr;

	// ========== スポーン設定 ==========

	float spawnInterval_ = 2.5f;     ///< スポーン間隔（全スポナー共通）
	float prevSpawnInterval_ = 2.5f; ///< 変更検出用

	// ========== スポーン処理 ==========

	/// <summary>
	/// 指定スポナーから敵を1体生成する
	/// </summary>
	void SpawnEnemy(No::Registry& registry, EnemySpawnerComponent* spawner);

	/// <summary>
	/// spawnInterval_ が変わったとき全スポナーの calculatedSpeed を再計算する
	/// </summary>
	void RecalculateAllSpeeds(No::Registry& registry);

#ifdef USE_IMGUI
	void DebugUI(No::Registry& registry);
#endif
};
