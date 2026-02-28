#pragma once
#include "engine/NoEngine.h"
#include "../MapData/StageData.h"
#include "../System/GameTimer.h"
#include "../Component/PlayerComponent.h"  // Direction enum を使用するため

// 前方宣言
class PlayerBulletSystem;
class EnemySpawnerSystem;

#ifdef USE_IMGUI
#include "../Editor/MapEditor.h"
#endif

/// <summary>
/// サンプルシーン
/// </summary>
class SampleScene : public No::IScene {
public:
	void Setup() override;

	/// <summary>
	/// ゲームタイマーを取得
	/// </summary>
	GameTimer* GetGameTimer() { return &gameTimer_; }

	/// <summary>
	/// 敵撃破数を増やす
	/// </summary>
	void IncrementEnemyKillCount() { enemyKillCount_++; }

	/// <summary>
	/// 敵撃破数を取得
	/// </summary>
	int GetEnemyKillCount() const { return enemyKillCount_; }

private:
	std::unique_ptr<NoEngine::Camera> camera_;
	NoEngine::Transform cameraTransform_{};

	// ========== ゲームタイマー ==========
	GameTimer gameTimer_;
	float lastRealDeltaTime_ = 0.0f;	// デバッグ表示用

	// ========== ステージ管理 ==========

	int  stageNumber_ = 0;		// ステージ番号(こ子は仮置きなんで０にしておく)
	int  enemyKillCount_ = 0;	// 敵撃破数
	int clearKillCount_ = 25;	// クリアに必要な撃破数

	/// <summary>
	/// ECS を全リセットし、指定ステージを再ロードする。
	/// </summary>
	void ReloadStage(int stageNumber);

	/// <summary>
	/// クリア・ゲームオーバー検出。NotSystemUpdate から毎フレーム呼ばれる。
	/// </summary>
	void UpdateGame(No::Registry& registry);

	/// <summary>
	/// Stage Control ImGui ウィンドウ（ステージ切り替え・エディタモード切り替え）
	/// </summary>
	void DebugStageControlUI(No::Registry& registry);

	// ========== エディタ管理 ==========

#ifdef USE_IMGUI
	std::unique_ptr<MapEditor> editor_;
	bool isEditorMode_ = false;	// エディタ使用フラグ
#endif

	/// <summary>
	/// エディタモード ON 時の処理。
	/// ゲームエンティティを全削除し、エディタを空のキャンバスで初期化する。
	/// </summary>
	void OnEnterEditorMode();

	/// <summary>
	/// エディタモード OFF 時の処理。現在のステージをリロードしてゲームに戻る。
	/// </summary>
	void OnExitEditorMode();


	void InitializeGrid(No::Registry& registry, const MapData::ConnectionMapData& mapData);
	void InitializePlayer(No::Registry& registry, int startX, int startY);
	void InitializeEnemy(No::Registry& registry, int startX, int startY);
	void InitializeSpawner(No::Registry& registry, int startX, int startY);
	void InitializeLight(No::Registry& registry);

	// ========== カメラ ==========

	/// <summary>
	/// 方向から回転を計算する（敵・スポナー共通）
	/// </summary>
	NoEngine::Math::Quaternion CalcDirectionRotation(Direction dir);

	/// <summary>
	/// スポナーの回転を更新する（SetupSpawners後に呼ぶ）
	/// </summary>
	void UpdateSpawnerRotations(No::Registry& registry);

	/// <summary>
	/// ステージのノード座標からマップ中心とZ距離を自動計算してカメラを設定する。
	/// Setup() と ReloadStage() の両方から呼ぶ。
	/// </summary>
	void SetupCameraForStage(const MapData::ConnectionMapData& mapData);

	// ========== システム更新 ==========

	void NotSystemUpdate() override;

	/// <summary>
	/// イテレータ安全版の一括削除
	/// </summary>
	void DestroyGameObject();

	// ========== システムへの参照 ==========

	/// SetupSpawners をステージロードのたびに呼ぶため、rawポインタを保持する。
	/// unique_ptr の所有権は IScene 側にある。
	EnemySpawnerSystem* spawnerSystem_ = nullptr;
};