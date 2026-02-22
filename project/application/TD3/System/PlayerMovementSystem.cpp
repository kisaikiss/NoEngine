#include "PlayerMovementSystem.h"
#include "../Component/GridCellComponent.h"
#include "../Component/PlayerTag.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

// キーコード定義（VK_*は本来Windows.hにあるが、衝突回避のため直接定義）
#define KEY_W 'W'
#define KEY_A 'A'
#define KEY_S 'S'
#define KEY_D 'D'

void PlayerMovementSystem::Update(No::Registry& registry, float deltaTime) {
    auto view = registry.View<PlayerComponent, PlayerTag, No::TransformComponent>();
    
    for (auto entity : view) {
        auto* player = registry.GetComponent<PlayerComponent>(entity);
        auto* transform = registry.GetComponent<No::TransformComponent>(entity);
        
        // 状態別の処理
        if (player->isOnNode) {
            HandleNodeInput(player, registry);
        } else if (player->isOnEdge) {
            HandleEdgeMovement(player, deltaTime, registry);
        }
        
        // Transform更新
        UpdateTransform(player, transform);
        
        // デバッグUI
        #ifdef USE_IMGUI
        ShowPlayerDebugUI(player);
        #endif
    }
}

void PlayerMovementSystem::HandleNodeInput(
    PlayerComponent* player,
    No::Registry& registry
) {
    // 全入力取得
    bool inputW = NoEngine::Input::Keyboard::IsPress(KEY_W);
    bool inputS = NoEngine::Input::Keyboard::IsPress(KEY_S);
    bool inputA = NoEngine::Input::Keyboard::IsPress(KEY_A);
    bool inputD = NoEngine::Input::Keyboard::IsPress(KEY_D);
    
    // 入力がなければ停止継続
    if (!inputW && !inputS && !inputA && !inputD) {
        return;
    }
    
    // 初期状態（向きなし）の場合
    if (player->lastDirection == Direction::None) {
        // 接続がある方向への入力で初期向きを決定
        if (inputW && CanMoveInDirection(registry, player->currentNodeX, 
            player->currentNodeY, Direction::Up, Direction::None)) {
            StartMovement(player, Direction::Up, registry);
        } else if (inputD && CanMoveInDirection(registry, player->currentNodeX, 
            player->currentNodeY, Direction::Right, Direction::None)) {
            StartMovement(player, Direction::Right, registry);
        } else if (inputS && CanMoveInDirection(registry, player->currentNodeX, 
            player->currentNodeY, Direction::Down, Direction::None)) {
            StartMovement(player, Direction::Down, registry);
        } else if (inputA && CanMoveInDirection(registry, player->currentNodeX, 
            player->currentNodeY, Direction::Left, Direction::None)) {
            StartMovement(player, Direction::Left, registry);
        }
        return;
    }
    
    // 通常移動（lastDirectionがある状態）
    // いずれかの方向に移動試行
    if (inputW && CanMoveInDirection(registry, player->currentNodeX, 
        player->currentNodeY, Direction::Up, player->lastDirection)) {
        StartMovement(player, Direction::Up, registry);
    } else if (inputD && CanMoveInDirection(registry, player->currentNodeX, 
        player->currentNodeY, Direction::Right, player->lastDirection)) {
        StartMovement(player, Direction::Right, registry);
    } else if (inputS && CanMoveInDirection(registry, player->currentNodeX, 
        player->currentNodeY, Direction::Down, player->lastDirection)) {
        StartMovement(player, Direction::Down, registry);
    } else if (inputA && CanMoveInDirection(registry, player->currentNodeX, 
        player->currentNodeY, Direction::Left, player->lastDirection)) {
        StartMovement(player, Direction::Left, registry);
    }
}

void PlayerMovementSystem::HandleEdgeMovement(
    PlayerComponent* player,
    float deltaTime,
    No::Registry& registry
) {
    // 現在方向の入力があるかチェック
    bool hasContinuousInput = false;
    
    switch (player->currentDirection) {
        case Direction::Up:
            hasContinuousInput = NoEngine::Input::Keyboard::IsPress(KEY_W);
            break;
        case Direction::Right:
            hasContinuousInput = NoEngine::Input::Keyboard::IsPress(KEY_D);
            break;
        case Direction::Down:
            hasContinuousInput = NoEngine::Input::Keyboard::IsPress(KEY_S);
            break;
        case Direction::Left:
            hasContinuousInput = NoEngine::Input::Keyboard::IsPress(KEY_A);
            break;
        default:
            break;
    }
    
    if (hasContinuousInput) {
        // 移動継続
        player->progressOnEdge += player->moveSpeed * deltaTime;
        
        // ノード到達チェック（高速移動対応）
        while (player->progressOnEdge >= 1.0f) {
            player->progressOnEdge -= 1.0f;
            OnReachNode(player, registry);
            
            // 停止したらループ脱出
            if (!player->isOnEdge) {
                break;
            }
        }
    } else {
        // 入力なし → エッジ途中で停止（progressOnEdge保持）
        StopMovement(player);
    }
}

void PlayerMovementSystem::OnReachNode(
    PlayerComponent* player,
    No::Registry& registry
) {
    // ノード更新
    player->currentNodeX = player->targetNodeX;
    player->currentNodeY = player->targetNodeY;
    player->isOnEdge = false;
    player->isOnNode = true;
    
    // 全入力取得
    bool inputW = NoEngine::Input::Keyboard::IsPress(KEY_W);
    bool inputS = NoEngine::Input::Keyboard::IsPress(KEY_S);
    bool inputA = NoEngine::Input::Keyboard::IsPress(KEY_A);
    bool inputD = NoEngine::Input::Keyboard::IsPress(KEY_D);
    
    Direction nextDir = Direction::None;
    
    // 進行方向別に曲がり優先判定
    // 曲がり（進行方向以外）を先にチェック、直進は後
    
    if (player->currentDirection == Direction::Right) {
        // 右進行中 → 上下優先、直進次点
        if (inputW && CanMoveInDirection(registry, player->currentNodeX, 
            player->currentNodeY, Direction::Up, player->lastDirection)) {
            nextDir = Direction::Up;
        } else if (inputS && CanMoveInDirection(registry, player->currentNodeX, 
            player->currentNodeY, Direction::Down, player->lastDirection)) {
            nextDir = Direction::Down;
        } else if (inputD && CanMoveInDirection(registry, player->currentNodeX, 
            player->currentNodeY, Direction::Right, player->lastDirection)) {
            nextDir = Direction::Right;
        }
    }
    else if (player->currentDirection == Direction::Left) {
        // 左進行中 → 上下優先
        if (inputW && CanMoveInDirection(registry, player->currentNodeX, 
            player->currentNodeY, Direction::Up, player->lastDirection)) {
            nextDir = Direction::Up;
        } else if (inputS && CanMoveInDirection(registry, player->currentNodeX, 
            player->currentNodeY, Direction::Down, player->lastDirection)) {
            nextDir = Direction::Down;
        } else if (inputA && CanMoveInDirection(registry, player->currentNodeX, 
            player->currentNodeY, Direction::Left, player->lastDirection)) {
            nextDir = Direction::Left;
        }
    }
    else if (player->currentDirection == Direction::Up) {
        // 上進行中 → 左右優先
        if (inputA && CanMoveInDirection(registry, player->currentNodeX, 
            player->currentNodeY, Direction::Left, player->lastDirection)) {
            nextDir = Direction::Left;
        } else if (inputD && CanMoveInDirection(registry, player->currentNodeX, 
            player->currentNodeY, Direction::Right, player->lastDirection)) {
            nextDir = Direction::Right;
        } else if (inputW && CanMoveInDirection(registry, player->currentNodeX, 
            player->currentNodeY, Direction::Up, player->lastDirection)) {
            nextDir = Direction::Up;
        }
    }
    else if (player->currentDirection == Direction::Down) {
        // 下進行中 → 左右優先
        if (inputA && CanMoveInDirection(registry, player->currentNodeX, 
            player->currentNodeY, Direction::Left, player->lastDirection)) {
            nextDir = Direction::Left;
        } else if (inputD && CanMoveInDirection(registry, player->currentNodeX, 
            player->currentNodeY, Direction::Right, player->lastDirection)) {
            nextDir = Direction::Right;
        } else if (inputS && CanMoveInDirection(registry, player->currentNodeX, 
            player->currentNodeY, Direction::Down, player->lastDirection)) {
            nextDir = Direction::Down;
        }
    }
    
    // 移動開始 or 停止
    if (nextDir != Direction::None) {
        player->lastDirection = player->currentDirection;
        StartMovement(player, nextDir, registry);
    } else {
        player->lastDirection = player->currentDirection;
        player->currentDirection = Direction::None;
        CheckDeadEnd(player, registry);
    }
}

void PlayerMovementSystem::StartMovement(
    PlayerComponent* player,
    Direction dir,
    No::Registry& registry
) {
    (void)registry; // 未使用パラメータ警告回避
    
    // 次のノード座標を計算
    GetNextNodeCoords(
        player->currentNodeX,
        player->currentNodeY,
        dir,
        player->targetNodeX,
        player->targetNodeY
    );
    
    // 移動状態を設定
    player->currentDirection = dir;
    player->isOnEdge = true;
    player->isOnNode = false;
    player->progressOnEdge = 0.0f;
    player->isAtDeadEnd = false;
}

void PlayerMovementSystem::StopMovement(PlayerComponent* player) {
    // エッジ途中で停止（progressOnEdge は保持）
    player->currentDirection = Direction::None;
    player->isOnEdge = false;
    player->isOnNode = false; // エッジ途中にいる
}

bool PlayerMovementSystem::CanMoveInDirection(
    No::Registry& registry,
    int nodeX, int nodeY,
    Direction dir,
    Direction lastDir
) {
    // グリッドセル取得
    auto* cell = GetGridCell(registry, nodeX, nodeY);
    if (!cell) return false;
    
    // 接続チェック
    if (!HasConnection(cell, dir)) return false;
    
    // 後退チェック
    Direction opposite = GetOppositeDirection(lastDir);
    
    // 初期状態なら後退判定スキップ
    if (lastDir == Direction::None) {
        return true;
    }
    
    // 後退判定
    if (dir == opposite) {
        // 例外: 行き止まりなら後退OK
        // 条件: 進行方向に接続なし & 反対方向のみ接続あり & 接続数1
        bool forwardBlocked = !HasConnection(cell, lastDir);
        bool onlyBackward = HasConnection(cell, opposite);
        
        int connectionCount = 0;
        if (cell->hasConnectionUp) connectionCount++;
        if (cell->hasConnectionRight) connectionCount++;
        if (cell->hasConnectionDown) connectionCount++;
        if (cell->hasConnectionLeft) connectionCount++;
        
        if (forwardBlocked && onlyBackward && connectionCount == 1) {
            return true; // 行き止まり例外
        }
        
        return false; // 通常の後退は不可
    }
    
    return true;
}

GridCellComponent* PlayerMovementSystem::GetGridCell(
    No::Registry& registry,
    int x, int y
) {
    auto view = registry.View<GridCellComponent>();
    for (auto entity : view) {
        auto* cell = registry.GetComponent<GridCellComponent>(entity);
        if (cell->gridX == x && cell->gridY == y) {
            return cell;
        }
    }
    return nullptr;
}

bool PlayerMovementSystem::HasConnection(
    const GridCellComponent* cell,
    Direction dir
) {
    switch (dir) {
        case Direction::Up:    return cell->hasConnectionUp;
        case Direction::Right: return cell->hasConnectionRight;
        case Direction::Down:  return cell->hasConnectionDown;
        case Direction::Left:  return cell->hasConnectionLeft;
        default:               return false;
    }
}

Direction PlayerMovementSystem::GetOppositeDirection(Direction dir) {
    switch (dir) {
        case Direction::Up:    return Direction::Down;
        case Direction::Right: return Direction::Left;
        case Direction::Down:  return Direction::Up;
        case Direction::Left:  return Direction::Right;
        default:               return Direction::None;
    }
}

void PlayerMovementSystem::GetNextNodeCoords(
    int x, int y,
    Direction dir,
    int& outX, int& outY
) {
    outX = x;
    outY = y;
    
    switch (dir) {
        case Direction::Up:    outY += 1; break;
        case Direction::Right: outX += 1; break;
        case Direction::Down:  outY -= 1; break;
        case Direction::Left:  outX -= 1; break;
        default: break;
    }
}

No::Vector3 PlayerMovementSystem::CalculateWorldPosition(
    const PlayerComponent* player
) {
    if (player->isOnNode) {
        // ノード上
        return No::Vector3{
            static_cast<float>(player->currentNodeX),
            static_cast<float>(player->currentNodeY),
            0.0f
        };
    } else {
        // エッジ上を線形補間
        No::Vector3 currentPos{
            static_cast<float>(player->currentNodeX),
            static_cast<float>(player->currentNodeY),
            0.0f
        };
        No::Vector3 targetPos{
            static_cast<float>(player->targetNodeX),
            static_cast<float>(player->targetNodeY),
            0.0f
        };
        
        return currentPos + (targetPos - currentPos) * player->progressOnEdge;
    }
}

void PlayerMovementSystem::UpdateTransform(
    PlayerComponent* player,
    No::TransformComponent* transform
) {
    transform->translate = CalculateWorldPosition(player);
}

void PlayerMovementSystem::CheckDeadEnd(
    PlayerComponent* player,
    No::Registry& registry
) {
    auto* cell = GetGridCell(registry, player->currentNodeX, player->currentNodeY);
    if (!cell) return;
    
    // 接続数カウント
    int connectionCount = 0;
    if (cell->hasConnectionUp) connectionCount++;
    if (cell->hasConnectionRight) connectionCount++;
    if (cell->hasConnectionDown) connectionCount++;
    if (cell->hasConnectionLeft) connectionCount++;
    
    // 接続数1 かつ 来た方向のみ = 行き止まり
    Direction opposite = GetOppositeDirection(player->lastDirection);
    bool onlyBackward = (connectionCount == 1) && HasConnection(cell, opposite);
    
    player->isAtDeadEnd = onlyBackward;
}

#ifdef USE_IMGUI
void PlayerMovementSystem::ShowPlayerDebugUI(PlayerComponent* player) {
    ImGui::Begin("Player Debug");
    
    // 位置情報
    ImGui::Text("=== Position ===");
    ImGui::Text("Current Node: (%d, %d)", 
        player->currentNodeX, player->currentNodeY);
    
    if (player->isOnEdge) {
        ImGui::Text("Target Node: (%d, %d)", 
            player->targetNodeX, player->targetNodeY);
        ImGui::ProgressBar(player->progressOnEdge, 
            ImVec2(-1, 0), "Progress");
    } else if (!player->isOnNode) {
        ImGui::Text("Stopped on Edge");
        ImGui::ProgressBar(player->progressOnEdge, 
            ImVec2(-1, 0), "Progress");
    }
    
    // 状態
    ImGui::Separator();
    ImGui::Text("=== State ===");
    ImGui::Text("On Node: %s", player->isOnNode ? "Yes" : "No");
    ImGui::Text("On Edge: %s", player->isOnEdge ? "Yes" : "No");
    ImGui::Text("At Dead End: %s", player->isAtDeadEnd ? "Yes" : "No");
    
    // 方向
    ImGui::Separator();
    ImGui::Text("=== Direction ===");
    ImGui::Text("Current: %s", DirectionToString(player->currentDirection));
    ImGui::Text("Last: %s", DirectionToString(player->lastDirection));
    
    // パラメータ
    ImGui::Separator();
    ImGui::DragFloat("Move Speed", &player->moveSpeed, 
        0.1f, 0.1f, 10.0f);
    
    ImGui::End();
}

const char* PlayerMovementSystem::DirectionToString(Direction dir) {
    switch (dir) {
        case Direction::None:  return "None";
        case Direction::Up:    return "Up";
        case Direction::Right: return "Right";
        case Direction::Down:  return "Down";
        case Direction::Left:  return "Left";
        default:               return "Unknown";
    }
}
#endif
