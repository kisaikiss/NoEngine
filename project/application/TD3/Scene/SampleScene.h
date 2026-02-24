#pragma once
#include "engine/NoEngine.h"
#include "../MapData/StageData.h"

/// <summary>
/// サンプルシーン
///
/// 【Stage6 変更点】
/// ・stageNumber_ メンバ追加（現在のステージ番号を保持）
/// ・ReloadStage(int) 追加（ECS全リセット→指定ステージを再初期化）
/// ・isEditorMode_ フラグ追加（Stage7 MapEditor との切り替え用、今は常に false）
/// ・UpdateGame() 追加（クリア/ゲームオーバー検出）
/// ・ImGui ウィンドウ "Stage Control" でステージ番号指定＋リロードが可能
/// </summary>
class SampleScene : public No::IScene {
public:
	void Setup() override;

private:
	std::unique_ptr<NoEngine::Camera> camera_;
	NoEngine::Transform cameraTransform_{};

	// ========== ステージ管理 ==========

	int  stageNumber_ = 1;     // 現在のステージ番号
	bool isEditorMode_ = false; // Stage7 で使用。今は常に false

	/// <summary>
	/// ECS を全リセットし、指定ステージを再ロードする。
	/// クリア・ゲームオーバー・ImGui からのリロード要求で呼ばれる。
	/// </summary>
	void ReloadStage(int stageNumber);

	/// <summary>
	/// ゲーム進行の状態チェック（クリア・ゲームオーバー検出）
	/// NotSystemUpdate から毎フレーム呼ばれる。
	/// </summary>
	void UpdateGame(No::Registry& registry);

	/// <summary>
	/// Stage Control ImGui ウィンドウ
	/// ステージ番号の指定・リロード・エディタモード切り替えボタンを持つ。
	/// </summary>
	void DebugStageControlUI(No::Registry& registry);

	// ========== 初期化ヘルパー ==========

	void InitializeGrid(No::Registry& registry, const MapData::ConnectionMapData& mapData);
	void InitializePlayer(No::Registry& registry, int startX, int startY);
	void InitializeEnemy(No::Registry& registry, int startX, int startY);
	void InitializeLight(No::Registry& registry);

	// ========== システム更新 ==========

	void NotSystemUpdate() override;

	/// <summary>
	/// イテレータ安全版の一括削除
	/// </summary>
	void DestroyGameObject();
};