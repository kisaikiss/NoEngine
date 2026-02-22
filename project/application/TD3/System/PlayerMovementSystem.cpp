#include "PlayerMovementSystem.h"
#include "../Component/GridCellComponent.h"
#include "../Component/PlayerTag.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

// キーコード定義
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
        switch (player->state) {
            case PlayerState::OnNode:
                HandleNodeInput(player, registry);
                break;
                
            case PlayerState::MovingOnEdge:
                HandleEdgeMovement(player, deltaTime, registry);
                break;
                
            case PlayerState::StoppedOnEdge:
                HandleStoppedOnEdge(player, registry);
                break;
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
        
        // 入力履歴を更新（進行方向以外のキーを記録）
        UpdateRecentInputs(player, deltaTime);
        
        // ノード到達チェック（高速移動対応）
        while (player->progressOnEdge >= 1.0f) {
            player->progressOnEdge -= 1.0f;
            OnReachNode(player, registry);
            
            // 停止したらループ脱出
            if (player->state != PlayerState::MovingOnEdge) {
                break;
            }
        }
    } else {
        // 入力なし → エッジ途中で停止（progressOnEdge保持）
        StopMovement(player);
    }
}

void PlayerMovementSystem::UpdateRecentInputs(
    PlayerComponent* player,
    float deltaTime
) {
    player->inputHistoryTime += deltaTime;
    
    // 現在押されているキーを取得
    bool inputW = NoEngine::Input::Keyboard::IsPress(KEY_W);
    bool inputS = NoEngine::Input::Keyboard::IsPress(KEY_S);
    bool inputA = NoEngine::Input::Keyboard::IsPress(KEY_A);
    bool inputD = NoEngine::Input::Keyboard::IsPress(KEY_D);
    
    // ========== 履歴から離されたキーを削除 ==========
    for (int i = 0; i < player->recentInputCount; ) {
        Direction dir = player->recentInputs[i];
        bool stillPressed = false;
        
        // このキーがまだ押されているかチェック
        switch (dir) {
            case Direction::Up:
                stillPressed = inputW;
                break;
            case Direction::Down:
                stillPressed = inputS;
                break;
            case Direction::Left:
                stillPressed = inputA;
                break;
            case Direction::Right:
                stillPressed = inputD;
                break;
            default:
                break;
        }
        
        if (!stillPressed) {
            // 離されたキーを履歴から削除（配列を詰める）
            for (int j = i; j < player->recentInputCount - 1; ++j) {
                player->recentInputs[j] = player->recentInputs[j + 1];
            }
            player->recentInputs[player->recentInputCount - 1] = Direction::None;
            player->recentInputCount--;
            // i はインクリメントしない（次の要素が詰められるため）
        } else {
            i++;  // 次の要素へ
        }
    }
    
    // ========== 進行方向以外のキーを記録 ==========
    auto addInput = [&](Direction dir) {
        // 既に記録されていないかチェック
        for (int i = 0; i < player->recentInputCount; ++i) {
            if (player->recentInputs[i] == dir) {
                return; // 既に記録済み
            }
        }
        // 新規追加（配列の末尾に追加 = 最新）
        if (player->recentInputCount < 4) {
            player->recentInputs[player->recentInputCount] = dir;
            player->recentInputCount++;
        }
    };
    
    // 進行方向以外のキーを記録
    if (inputW && player->currentDirection != Direction::Up) {
        addInput(Direction::Up);
    }
    if (inputS && player->currentDirection != Direction::Down) {
        addInput(Direction::Down);
    }
    if (inputA && player->currentDirection != Direction::Left) {
        addInput(Direction::Left);
    }
    if (inputD && player->currentDirection != Direction::Right) {
        addInput(Direction::Right);
    }
    
    // ========== 履歴ウィンドウを超えたらクリア ==========
    if (player->inputHistoryTime > player->inputHistoryWindow) {
        player->recentInputCount = 0;
        player->inputHistoryTime = 0.0f;
        for (int i = 0; i < 4; ++i) {
            player->recentInputs[i] = Direction::None;
        }
    }
}

void PlayerMovementSystem::HandleStoppedOnEdge(
    PlayerComponent* player,
    No::Registry& registry
) {
    (void)registry; // 未使用パラメータ警告回避
    
    // actualMovingDirection が None なら何もしない（初期状態）
    if (player->actualMovingDirection == Direction::None) {
        return;
    }
    
    // 実際に移動していた方向のキー入力をチェック
    bool hasContinuousInput = false;
    
    switch (player->actualMovingDirection) {
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
        // 移動再開（progressOnEdge保持、同じ方向に継続）
        player->state = PlayerState::MovingOnEdge;
        player->currentDirection = player->actualMovingDirection;
    }
    
    // 他の入力は完全無視
}

void PlayerMovementSystem::OnReachNode(
    PlayerComponent* player,
    No::Registry& registry
) {
    // ノード更新
    player->currentNodeX = player->targetNodeX;
    player->currentNodeY = player->targetNodeY;
    player->state = PlayerState::OnNode;
    
    // ========== ノード到達時に「来た方向」を記録 ==========
    if (player->currentDirection != Direction::None) {
        player->lastDirection = player->currentDirection;
    }
    
    Direction nextDir = Direction::None;
    
    // ========== 1. 入力履歴から最新の入力を優先的にチェック =========
    // 逆順（最新から）でチェック
    for (int i = player->recentInputCount - 1; i >= 0; --i) {
        Direction dir = player->recentInputs[i];
        
        if (CanMoveInDirection(registry, player->currentNodeX, 
            player->currentNodeY, dir, player->lastDirection)) {
            nextDir = dir;
            break; // 最新の移動可能な方向を採用
        }
    }
    
    // ========== 2. 履歴になければ現在の入力をチェック =========
    if (nextDir == Direction::None) {
        // 全入力取得
        bool inputW = NoEngine::Input::Keyboard::IsPress(KEY_W);
        bool inputS = NoEngine::Input::Keyboard::IsPress(KEY_S);
        bool inputA = NoEngine::Input::Keyboard::IsPress(KEY_A);
        bool inputD = NoEngine::Input::Keyboard::IsPress(KEY_D);
        
        // 進行方向別に曲がり優先判定
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
    }
    
    // ========== 入力履歴をクリア =========
    player->recentInputCount = 0;
    player->inputHistoryTime = 0.0f;
    for (int i = 0; i < 4; ++i) {
        player->recentInputs[i] = Direction::None;
    }
    
    // ========== 移動開始 or 停止 =========
    if (nextDir != Direction::None) {
        StartMovement(player, nextDir, registry);
    } else {
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
    
    // 初回移動時のみ lastDirection を設定
    // 2回目以降は OnReachNode で更新される
    if (player->lastDirection == Direction::None) {
        player->lastDirection = dir;
    }
    
    // 実際の移動方向を記録（エッジ停止時の再開に使用）
    player->actualMovingDirection = dir;
    
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
    player->state = PlayerState::MovingOnEdge;
    player->progressOnEdge = 0.0f;
    player->isAtDeadEnd = false;
}

void PlayerMovementSystem::StopMovement(PlayerComponent* player) {
    // エッジ途中で停止（progressOnEdgeは保持）
    player->currentDirection = Direction::None;
    player->state = PlayerState::StoppedOnEdge;
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
    if (player->state == PlayerState::OnNode) {
        // ノード上
        return No::Vector3{
            static_cast<float>(player->currentNodeX),
            static_cast<float>(player->currentNodeY),
            0.0f
        };
    } else {
        // エッジ上を線形補間（MovingOnEdge or StoppedOnEdge）
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
    
    if (player->state != PlayerState::OnNode) {
        ImGui::Text("Target Node: (%d, %d)", 
            player->targetNodeX, player->targetNodeY);
        ImGui::ProgressBar(player->progressOnEdge, 
            ImVec2(-1, 0), "Progress");
    }
    
    // 状態
    ImGui::Separator();
    ImGui::Text("=== State ===");
    ImGui::Text("State: %s", StateToString(player->state));
    ImGui::Text("At Dead End: %s", player->isAtDeadEnd ? "Yes" : "No");
    
    // 方向
    ImGui::Separator();
    ImGui::Text("=== Direction ===");
    ImGui::Text("Current: %s", DirectionToString(player->currentDirection));
    ImGui::Text("Last: %s", DirectionToString(player->lastDirection));
    ImGui::Text("Actual Moving: %s", DirectionToString(player->actualMovingDirection));
    
    // 入力履歴
    ImGui::Separator();
    ImGui::Text("=== Input History ===");
    ImGui::Text("Count: %d", player->recentInputCount);
    for (int i = 0; i < player->recentInputCount; ++i) {
        ImGui::Text("  [%d] %s", i, DirectionToString(player->recentInputs[i]));
    }
    ImGui::Text("History Time: %.2f", player->inputHistoryTime);
    
    // パラメータ
    ImGui::Separator();
    ImGui::DragFloat("Move Speed", &player->moveSpeed, 
        0.1f, 0.1f, 10.0f);
    ImGui::DragFloat("Input History Window", &player->inputHistoryWindow,
        0.01f, 0.0f, 1.0f);
    
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

const char* PlayerMovementSystem::StateToString(PlayerState state) {
    switch (state) {
        case PlayerState::OnNode:        return "OnNode";
        case PlayerState::MovingOnEdge:  return "MovingOnEdge";
        case PlayerState::StoppedOnEdge: return "StoppedOnEdge";
        default:                         return "Unknown";
    }
}
#endif
