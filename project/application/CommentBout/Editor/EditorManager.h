#pragma once
#include "engine/NoEngine.h"
#include "application/CommentBout/Editor/RailCameraEditor.h"
#include "application/CommentBout/Editor/GameEventEditor.h"
#include "application/CommentBout/FieldObject/Editor/FieldObjectEditor.h"
#include "application/CommentBout/Data/EnemyConfig.h"
#include <vector>

/// <summary>
/// ゲームエディタ全体を統括するマネージャ。
/// エディタモードの概念はなく、常時 ImGui ウィンドウを表示する。
/// ゲームポーズはウィンドウ上のチェックボックスで手動制御。
/// GameScene::NotSystemUpdate() から DrawImGui() を毎フレーム呼ぶ。
/// </summary>
class EditorManager {
public:
    void Initialize(No::Registry& registry, No::Entity railCameraEntity);
    void DrawImGui(No::Registry& registry);

    FieldObjectEditor& GetFieldObjectEditor() { return fieldObjectEditor_; }

private:
    // スポーンデバッグエンティティ管理
    void CreateSpawnDebugEntities(No::Registry& registry);
    void DestroySpawnDebugEntities(No::Registry& registry);
    int  ComputeSpawnEventSignature(No::Registry& registry) const;

    // ステージ一括 IO
    void LoadAll(No::Registry& registry);
    void SaveAll(No::Registry& registry);

    // タブ描画
    void DrawRailTab(No::Registry& registry);
    void DrawEventTab(No::Registry& registry);
    void DrawFieldTab(No::Registry& registry);
    void DrawEnemyTab(No::Registry& registry);
    void DrawFieldTypeDefaultsTab(No::Registry& registry);
    void DrawSpeechBubbleTab(No::Registry& registry);

    void ApplyEnemyPresetsToAliveEnemies(No::Registry& registry);

    No::Entity railCameraEntity_ = No::nullEntity;

    RailCameraEditor  railCameraEditor_;
    GameEventEditor   gameEventEditor_;
    FieldObjectEditor fieldObjectEditor_;

    EnemyConfigMap enemyPresets_;
    bool enemyPresetsLoaded_ = false;

    char stageNameBuffer_[64] = "";

    bool isPauseEnabled_     = false;
    bool showSpawnDebug_     = false;
    int  lastSpawnDebugSig_  = -1;

    std::vector<No::Entity> spawnDebugEntities_;
};
