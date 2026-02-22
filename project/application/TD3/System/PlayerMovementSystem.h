#pragma once
#include "engine/NoEngine.h"
#include "../Component/PlayerComponent.h"
#include "../Component/GridCellComponent.h"

// プレイヤー移動システム
// 入力処理、移動判定、ノード到達処理を担当
class PlayerMovementSystem : public No::ISystem {
public:
    void Update(No::Registry& registry, float deltaTime) override;
    
private:
    // ========== ノード上の入力処理 ==========
    void HandleNodeInput(
        PlayerComponent* player,
        No::Registry& registry
    );
    
    // ========== エッジ上の移動処理 ==========
    void HandleEdgeMovement(
        PlayerComponent* player,
        float deltaTime,
        No::Registry& registry
    );
    
    // ========== エッジ上で停止中の処理 ==========
    void HandleStoppedOnEdge(
        PlayerComponent* player,
        No::Registry& registry
    );
    
    // ========== 入力履歴の更新 ==========
    void UpdateRecentInputs(
        PlayerComponent* player,
        float deltaTime
    );
    
    // ========== ノード到達処理 ==========
    void OnReachNode(
        PlayerComponent* player,
        No::Registry& registry
    );
    
    // ========== 移動開始 ==========
    void StartMovement(
        PlayerComponent* player,
        Direction dir,
        No::Registry& registry
    );
    
    // ========== 移動停止 ==========
    void StopMovement(PlayerComponent* player);
    
    // ========== 移動可能判定 ==========
    // 指定方向に移動できるかチェック（接続・後退判定込み）
    bool CanMoveInDirection(
        No::Registry& registry,
        int nodeX, int nodeY,
        Direction dir,
        Direction lastDir
    );
    
    // ========== グリッドセル取得 ==========
    GridCellComponent* GetGridCell(
        No::Registry& registry,
        int x, int y
    );
    
    // ========== 接続チェック ==========
    bool HasConnection(
        const GridCellComponent* cell,
        Direction dir
    );
    
    // ========== 反対方向取得 ==========
    Direction GetOppositeDirection(Direction dir);
    
    // ========== 次ノード座標計算 ==========
    void GetNextNodeCoords(
        int x, int y,
        Direction dir,
        int& outX, int& outY
    );
    
    // ========== ワールド座標計算 ==========
    No::Vector3 CalculateWorldPosition(
        const PlayerComponent* player
    );
    
    // ========== Transform更新 ==========
    void UpdateTransform(
        PlayerComponent* player,
        No::TransformComponent* transform
    );
    
    // ========== 行き止まりチェック ==========
    void CheckDeadEnd(
        PlayerComponent* player,
        No::Registry& registry
    );
    
    // ========== デバッグUI ==========
    #ifdef USE_IMGUI
    void ShowPlayerDebugUI(PlayerComponent* player);
    const char* DirectionToString(Direction dir);
    const char* StateToString(PlayerState state);
    #endif
};
