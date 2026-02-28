#pragma once
#include "engine/NoEngine.h"
#include "../Component/EnemySpawnerComponent.h"

// 前方宣言
class GameTimer;

/// <summary>
/// 敵スポナーシステム
///
/// 【初期化（SetupSpawners）】
///   ステージロード後に SampleScene から呼ぶ。
///   各スポナーの spawnDirection を自動検出し、chainCount と calculatedSpeed を計算する。
///
/// 【更新（Update）】
///   GameTimer を参照してスポーンタイマーを進める。
///   タイマーが spawnInterval_ に達したら敵を生成してタイマーをリセットする。
///   生成した敵は isSpawning=true、spawningSpeed=calculatedSpeed、
///   lastDirection=spawnDirection の逆方向で初期化される。
///
/// 【ImGui】
///   - スポーン間隔スライダー（全スポナー共通。変更時に calculatedSpeed を再計算）
///   - 各スポナーの残り時間 ProgressBar と数値
///   - 自動計算された速度の読み取り表示
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

	float spawnInterval_ = 2.5f;     ///< スポーン間隔（全スポナー共通, ImGui で変更可）
	float prevSpawnInterval_ = 2.5f; ///< 変更検出用

	// ========== スポーン処理 ==========

	/// <summary>
	/// 指定スポナーから敵を 1 体生成する
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
