#pragma once
#include "engine/NoEngine.h"
#include "../MapData/StageData.h"

#ifdef USE_IMGUI
#include "../Editor/MapEditor.h"
#endif
/// <summary>
/// サンプルシーン
/// </summary>
class SampleScene : public No::IScene {
public:
	void Setup() override;

private:
	std::unique_ptr<NoEngine::Camera> camera_;
	NoEngine::Transform cameraTransform_{};

	// ========== ステージ管理 ==========

	int  stageNumber_ = 1;		// 現在のステージ番号

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
	void InitializeLight(No::Registry& registry);

	// ========== システム更新 ==========

	void NotSystemUpdate() override;

	/// <summary>
	/// イテレータ安全版の一括削除
	/// </summary>
	void DestroyGameObject();
};