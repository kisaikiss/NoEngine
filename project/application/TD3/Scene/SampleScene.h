#pragma once
#include "engine/NoEngine.h"
#include "../MapData/StageData.h"
#include "../System/GameTimer.h"
#include "../Component/PlayerComponent.h"  // Direction enum ��g�p���邽��

// �O���錾
class PlayerBulletSystem;
class EnemySpawnerSystem;

#ifdef USE_IMGUI
#include "../Editor/MapEditor.h"
#endif

/// <summary>
/// �T���v���V�[��
/// </summary>
class SampleScene : public No::IScene {
public:
	void Setup() override;

	/// <summary>
	/// �Q�[���^�C�}�[��擾
	/// </summary>
	GameTimer* GetGameTimer() { return &gameTimer_; }

	/// <summary>
	/// �G���j���𑝂₷
	/// </summary>
	void IncrementEnemyKillCount() { enemyKillCount_++; }

	/// <summary>
	/// �G���j����擾
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
	/// �C�e���[�^���S�ł̈ꊇ�폜
	/// </summary>
	void DestroyGameObject();

	// ========== �V�X�e���ւ̎Q�� ==========

	/// SetupSpawners ��X�e�[�W���[�h�̂��тɌĂԂ��߁Araw�|�C���^��ێ�����B
	/// unique_ptr �̏��L���� IScene ���ɂ���B
	EnemySpawnerSystem* spawnerSystem_ = nullptr;
};