#include "PlayerMovementSystem.h"
#include "../Component/GridCellComponent.h"
#include "../Component/PlayerTag.h"
#include "../Component/PlayerBulletComponent.h"
#include "../Component/AmmoItemComponent.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

// ============================================================
// キーコード定義
// ============================================================
#define KEY_W 'W'
#define KEY_A 'A'
#define KEY_S 'S'
#define KEY_D 'D'
#define KEY_SPACE VK_SPACE

// ============================================================
//  Update
// ============================================================

void PlayerMovementSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<PlayerComponent, PlayerTag, No::TransformComponent>();

	for (auto entity : view) {
		auto* player = registry.GetComponent<PlayerComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);

		// 弾丸発射処理
		HandleBulletFire(player, registry, transform->translate);

		// 状態別の処理
		switch (player->state) {
		case PlayerState::OnNode:
			// HandleIntersection は OnReachNode 内でのみ呼ぶ。
			// ここで呼ぶと毎フレーム実行されてしまうため削除。
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

// ============================================================
//  HandleNodeInput
// ============================================================

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

// ============================================================
//  HandleEdgeMovement
// ============================================================

void PlayerMovementSystem::HandleEdgeMovement(
	PlayerComponent* player,
	float deltaTime,
	No::Registry& registry
) {
	// ---- 現在の進行方向キーが押されているか（原文のまま） ----
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

	///終点近傍の自動前進チェック
	// progress が NEAR_END_THRESHOLD 以上のとき、ターゲットノードで有効な方向キーが押されているだけでも前進を許可する。
	// どの方向に進むかは OnReachNode の recentInputs（最後押し優先）に任せる。
	// すでに hasContinuousInput が true の場合は不要なので else で判定する。
	if (!hasContinuousInput && player->progressOnEdge >= NEAR_END_THRESHOLD) {
		// futureLastDir: ターゲットノード到達後の lastDirection = 現在の進行方向
		if (HasValidNearEndInput(player, registry, player->currentDirection)) {
			hasContinuousInput = true;
		}
	}

	if (hasContinuousInput) {
		// 移動継続
		player->progressOnEdge += player->moveSpeed * deltaTime;

		// 入力履歴を更新（進行方向以外のキーを記録）
		// ここで終点近傍キーも recentInputs に記録される
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
		// 入力なし,エッジ途中で停止（progressOnEdge保持）
		StopMovement(player);
	}
}

// ============================================================
//  HandleStoppedOnEdge
// ============================================================

void PlayerMovementSystem::HandleStoppedOnEdge(
	PlayerComponent* player,
	No::Registry& registry
) {
	// actualMovingDirection が None なら何もしない（初期状態）
	if (player->actualMovingDirection == Direction::None) {
		return;
	}

	// 来た方向のキーで移動再開
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

	///終点近傍の再開チェック
	// 来た方向キーが押されておらず、かつ progress が NEAR_END_THRESHOLD 以上のとき、ターゲットノードで有効な方向キーが押されているだけでも再開を許可する。
	// どの方向に進むかは OnReachNode の recentInputs（最後押し優先）に任せる。
	if (!hasContinuousInput && player->progressOnEdge >= NEAR_END_THRESHOLD) {
		// futureLastDir: ターゲットノード到達後の lastDirection = 実際に移動していた方向
		if (HasValidNearEndInput(player, registry, player->actualMovingDirection)) {
			hasContinuousInput = true;
		}
	}

	if (hasContinuousInput) {
		// 移動再開（progressOnEdge保持、同じ方向に継続）
		player->state = PlayerState::MovingOnEdge;
		player->currentDirection = player->actualMovingDirection;
	}

	// 有効でない方向キーは完全無視
}

// ============================================================
//  HasValidNearEndInput
// ============================================================

bool PlayerMovementSystem::HasValidNearEndInput(
	PlayerComponent* player,
	No::Registry& registry,
	Direction futureLastDir
) {
	// 現在の入力を取得
	bool bW = NoEngine::Input::Keyboard::IsPress(KEY_W);
	bool bS = NoEngine::Input::Keyboard::IsPress(KEY_S);
	bool bA = NoEngine::Input::Keyboard::IsPress(KEY_A);
	bool bD = NoEngine::Input::Keyboard::IsPress(KEY_D);

	// 各キーについて「ターゲットノードで有効か」を CanMoveInDirection で判定する
	// ※方向は返さない。どの方向に進むかは OnReachNode の recentInputs に任せる
	if (bW && CanMoveInDirection(registry,
		player->targetNodeX, player->targetNodeY,
		Direction::Up, futureLastDir)) {
		return true;
	}
	if (bS && CanMoveInDirection(registry,
		player->targetNodeX, player->targetNodeY,
		Direction::Down, futureLastDir)) {
		return true;
	}
	if (bA && CanMoveInDirection(registry,
		player->targetNodeX, player->targetNodeY,
		Direction::Left, futureLastDir)) {
		return true;
	}
	if (bD && CanMoveInDirection(registry,
		player->targetNodeX, player->targetNodeY,
		Direction::Right, futureLastDir)) {
		return true;
	}

	return false;
}

// ============================================================
//  UpdateRecentInputs
// ============================================================

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

// ============================================================
//  OnReachNode
// ============================================================

void PlayerMovementSystem::OnReachNode(
	PlayerComponent* player,
	No::Registry& registry
) {
	// ノード更新
	player->currentNodeX = player->targetNodeX;
	player->currentNodeY = player->targetNodeY;
	player->state = PlayerState::OnNode;

	/// ========== ノード到達時に「来た方向」を記録 ==========

	if (player->currentDirection != Direction::None) {
		player->lastDirection = player->currentDirection;
	}

	// 交差点検出
	HandleIntersection(player, registry);

	Direction nextDir = Direction::None;


	/// ========== 1. 入力履歴から最新の入力を優先的にチェック ==========
	// 逆順（最新から）でチェックすることで最後に押したキーが優先される
	for (int i = player->recentInputCount - 1; i >= 0; --i) {
		Direction dir = player->recentInputs[i];

		if (CanMoveInDirection(registry, player->currentNodeX,
			player->currentNodeY, dir, player->lastDirection)) {
			nextDir = dir;
			break; // 最新の移動可能な方向を採用
		}
	}

	/// ========== 2. 履歴になければ現在の入力をチェック ==========
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
		} else if (player->currentDirection == Direction::Left) {
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
		} else if (player->currentDirection == Direction::Up) {
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
		} else if (player->currentDirection == Direction::Down) {
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

	/// ========== 入力履歴をクリア ========== ///
	player->recentInputCount = 0;
	player->inputHistoryTime = 0.0f;
	for (int i = 0; i < 4; ++i) {
		player->recentInputs[i] = Direction::None;
	}

	/// ========== 移動開始 or 停止 ========== ///
	if (nextDir != Direction::None) {
		StartMovement(player, nextDir, registry);
	} else {
		player->currentDirection = Direction::None;
		CheckDeadEnd(player, registry);
	}
}

// ============================================================
//  StartMovement
// ============================================================

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

// ============================================================
//  StopMovement
// ============================================================

void PlayerMovementSystem::StopMovement(PlayerComponent* player) {
	// エッジ途中で停止（progressOnEdgeは保持）
	player->currentDirection = Direction::None;
	player->state = PlayerState::StoppedOnEdge;
}

// ============================================================
//  CanMoveInDirection
// ============================================================

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
		// 行き止まりなら例外的に後退OK
		// 条件:進行方向に接続なし & 反対方向のみ接続あり & 接続数1
		bool forwardBlocked = !HasConnection(cell, lastDir);
		bool onlyBackward = HasConnection(cell, opposite);

		int connectionCount = 0;
		if (cell->hasConnectionUp)    connectionCount++;
		if (cell->hasConnectionRight) connectionCount++;
		if (cell->hasConnectionDown)  connectionCount++;
		if (cell->hasConnectionLeft)  connectionCount++;

		if (forwardBlocked && onlyBackward && connectionCount == 1) {
			return true; // 行き止まり例外
		}

		return false; // 通常の後退は不可
	}

	return true;
}

// ============================================================
//  GetGridCell
// ============================================================

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

// ============================================================
//  HasConnection
// ============================================================

bool PlayerMovementSystem::HasConnection(
	const GridCellComponent* cell,
	Direction dir
) {
	switch (dir) {
	case Direction::Up:		return cell->hasConnectionUp;
	case Direction::Right:	return cell->hasConnectionRight;
	case Direction::Down:	return cell->hasConnectionDown;
	case Direction::Left:	return cell->hasConnectionLeft;
	default:				return false;
	}
}

// ============================================================
//  GetOppositeDirection
// ============================================================

Direction PlayerMovementSystem::GetOppositeDirection(Direction dir) {
	switch (dir) {
	case Direction::Up:		return Direction::Down;
	case Direction::Right:	return Direction::Left;
	case Direction::Down:	return Direction::Up;
	case Direction::Left:	return Direction::Right;
	default:				return Direction::None;
	}
}

// ============================================================
//  GetNextNodeCoords
// ============================================================

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

// ============================================================
//  CalculateWorldPosition
// ============================================================

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

// ============================================================
//  UpdateTransform
// ============================================================

void PlayerMovementSystem::UpdateTransform(
	PlayerComponent* player,
	No::TransformComponent* transform
) {
	// 位置を更新する
	transform->translate = CalculateWorldPosition(player);

	// 回転を更新する
	// actualMovingDirectionで、停止中も最後に動いた方向を向き続ける
	// actualMovingDirection が None（一度も動いていない）のときは初期向き（Z-）になる
	transform->rotation = CalcDirectionRotation(player->actualMovingDirection);
}

// ============================================================
//  CalcDirectionRotation
// ============================================================
NoEngine::Math::Quaternion PlayerMovementSystem::CalcDirectionRotation(Direction dir) {

	NoEngine::Math::Quaternion q;
	switch (dir) {
	case Direction::None:
		// 初期向き：Z- を向く（カメラ側） = Y 軸 +180°
		q.FromAxisAngle(
			NoEngine::Math::Vector3{ 0.0f, 1.0f, 0.0f }, PI);
		break;

	case Direction::Up:
		q.FromAxisAngle(
			NoEngine::Math::Vector3{ 1.0f, 0.0f, 0.0f }, -PI * 0.5f);
		break;

	case Direction::Down:
		q.FromAxisAngle(
			NoEngine::Math::Vector3{ 1.0f, 0.0f, 0.0f }, PI * 0.5f);
		break;

	case Direction::Right:
		q.FromAxisAngle(
			NoEngine::Math::Vector3{ 0.0f, 1.0f, 0.0f }, PI * 0.5f);
		break;

	case Direction::Left:
		q.FromAxisAngle(
			NoEngine::Math::Vector3{ 0.0f, 1.0f, 0.0f }, -PI * 0.5f);
		break;

	default:
		q = NoEngine::Math::Quaternion::IDENTITY;
		break;
	}
	return q;
}

// ============================================================
//  CheckDeadEnd  
// ============================================================

void PlayerMovementSystem::CheckDeadEnd(
	PlayerComponent* player,
	No::Registry& registry
) {
	auto* cell = GetGridCell(registry, player->currentNodeX, player->currentNodeY);
	if (!cell) return;

	// 接続数カウント
	int connectionCount = 0;
	if (cell->hasConnectionUp)    connectionCount++;
	if (cell->hasConnectionRight) connectionCount++;
	if (cell->hasConnectionDown)  connectionCount++;
	if (cell->hasConnectionLeft)  connectionCount++;

	// 接続数1 かつ 来た方向のみ = 行き止まり
	Direction opposite = GetOppositeDirection(player->lastDirection);
	bool onlyBackward = (connectionCount == 1) && HasConnection(cell, opposite);

	player->isAtDeadEnd = onlyBackward;
}

// ============================================================
//  デバッグ UI
// ============================================================

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

	// 弾薬情報
	ImGui::Separator();
	ImGui::Text("=== Ammo ===");
	ImGui::Text("Bullets: %d / %d", player->currentBullets, player->maxBullets);

	// 最大弾数の編集（1〜99）
	ImGui::DragInt("Max Bullets", &player->maxBullets, 1, 1, 99);

	// 現在の弾数の編集（0〜最大弾数）
	ImGui::DragInt("Current Bullets", &player->currentBullets, 1, 0, player->maxBullets);

	// 現在の弾数を最大にするボタン
	if (ImGui::Button("Fill Ammo")) {
		player->currentBullets = player->maxBullets;
	}

	// 弾の速度の編集
	ImGui::DragFloat("Bullet Speed", &player->bulletSpeed, 0.1f, 1.0f, 20.0f);

	// 移動パラメータ
	ImGui::Separator();
	ImGui::DragFloat("Move Speed", &player->moveSpeed, 0.1f, 0.1f, 10.0f);
	ImGui::DragFloat("Input History Window", &player->inputHistoryWindow, 0.01f, 0.0f, 1.0f);

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
// ============================================================
//  弾丸発射
// ============================================================

void PlayerMovementSystem::HandleBulletFire(
	PlayerComponent* player,
	No::Registry& registry,
	const No::Vector3& playerPosition
) {
	// スペースキーのトリガー入力をチェック
	if (!NoEngine::Input::Keyboard::IsTrigger(KEY_SPACE)) {
		return;
	}

	// 弾数チェック
	if (player->currentBullets <= 0) {
		return;
	}

	// 移動方向がない場合は発射しない
	if (player->actualMovingDirection == Direction::None) {
		return;
	}

	// ---- 発射前の前方接続チェック ----
	// 現在のノードから actualMovingDirection 方向への接続がない場合は発射しない。
	// 例：行き止まりノードで前方に接続がない場合、弾を生成してもグリッドのない空間に飛び出してしまうためここで弾消費なしにブロックする。
	auto* cell = GetGridCell(registry, player->currentNodeX, player->currentNodeY);
	if (!cell) {
		return;
	}
	if (!HasConnection(cell, player->actualMovingDirection)) {
		return;
	}

	// 弾数を消費
	player->currentBullets--;

	// 弾丸エンティティを生成
	auto bulletEntity = registry.GenerateEntity();

	// PlayerBulletComponent追加
	auto* bullet = registry.AddComponent<PlayerBulletComponent>(bulletEntity);
	bullet->direction = DirectionToVector(player->actualMovingDirection);

	// 発射元ノード座標を記録する
	// PlayerBulletSystem はこのノードのみグリッド判定をスキップし、それ以降のノードで前方接続チェックを行う。
	// プレイヤーがエッジ途中にいる場合でも currentNodeX/Y（直前に通過したノード）を使う。
	// 発射時点で弾がそのノードより先にいるケースでは、そのノードまでの距離が NODE_DETECT_THRESHOLD を超えているため自然に無視される。
	bullet->startNodeX = player->currentNodeX;
	bullet->startNodeY = player->currentNodeY;
	bullet->speed = player->bulletSpeed;
	bullet->maxDistance = 20.0f;

	// Transform追加
	auto* transform = registry.AddComponent<No::TransformComponent>(bulletEntity);
	transform->translate = playerPosition;
	transform->scale = { 0.2f, 0.2f, 0.2f };

	// Mesh追加
	auto* mesh = registry.AddComponent<No::MeshComponent>(bulletEntity);
	auto* material = registry.AddComponent<No::MaterialComponent>(bulletEntity);

	NoEngine::Asset::ModelLoader::LoadModel(
		"PlayerBullet",
		"resources/game/td_3105/Model/ball/ball.obj",
		mesh
	);

	material->materials = NoEngine::Asset::ModelLoader::GetMaterial("PlayerBullet");
	material->color = { 0.0f, 0.0f, 0.0f, 1.0f }; // 黒色
	material->psoName = L"Renderer : Default PSO";
	material->psoId = NoEngine::Render::GetPSOID(material->psoName);
	material->rootSigId = NoEngine::Render::GetRootSignatureID(material->psoName);
}

No::Vector3 PlayerMovementSystem::DirectionToVector(Direction dir) {
	switch (dir) {
	case Direction::Up:    return { 0.0f, 1.0f, 0.0f };
	case Direction::Right: return { 1.0f, 0.0f, 0.0f };
	case Direction::Down:  return { 0.0f, -1.0f, 0.0f };
	case Direction::Left:  return { -1.0f, 0.0f, 0.0f };
	default:               return { 0.0f, 0.0f, 0.0f };
	}
}

// ============================================================
//  交差点検出と弾薬配置
// ============================================================

bool PlayerMovementSystem::IsIntersection(const GridCellComponent* cell) {
	if (!cell) return false;

	int connectionCount = 0;
	if (cell->hasConnectionUp)    connectionCount++;
	if (cell->hasConnectionRight) connectionCount++;
	if (cell->hasConnectionDown)  connectionCount++;
	if (cell->hasConnectionLeft)  connectionCount++;

	return connectionCount >= 3;
}

void PlayerMovementSystem::HandleIntersection(
	PlayerComponent* player,
	No::Registry& registry
) {
	// 現在のノードを取得
	auto* cell = GetGridCell(registry, player->currentNodeX, player->currentNodeY);

	// 交差点でなければ何もしない（接続数 3 以上を交差点と定義）
	if (!IsIntersection(cell)) {
		return;
	}

	// ---- 弾薬アイテムの状態で処理を分岐 ----
	//【弾薬がない場合】
	//→ アイテムを新規配置する（canPickup = false で配置、まだ回収不可）
	//【弾薬がある場合】
	//→ 回収可能にする（canPickup = true）
	//→ AmmoItemSystem が同フレーム内でプレイヤー座標と照合して回収する
	// 
	//1回目の通過: 弾薬がない → 配置
	//2回目の通過: 弾薬がある → 回収可能化 → AmmoItemSystem が回収
	//3回目の通過: 弾薬がない（回収済み）→ 再配置

	if (!HasAmmoAtPosition(registry, player->currentNodeX, player->currentNodeY)) {
		// 弾薬なし → 新規配置
		CreateAmmoItem(registry, player->currentNodeX, player->currentNodeY);
	} else {
		// 弾薬あり → 回収可能にする
		EnableAmmoPickup(registry, player->currentNodeX, player->currentNodeY);
	}
}

bool PlayerMovementSystem::HasAmmoAtPosition(
	No::Registry& registry,
	int gridX,
	int gridY
) {
	auto view = registry.View<AmmoItemComponent>();
	if (view.Empty()) {
		return false;
	}

	for (auto entity : view) {
		auto* ammo = registry.GetComponent<AmmoItemComponent>(entity);
		if (ammo->gridX == gridX && ammo->gridY == gridY) {
			return true;
		}
	}
	return false;
}

void PlayerMovementSystem::CreateAmmoItem(
	No::Registry& registry,
	int gridX,
	int gridY
) {
	auto entity = registry.GenerateEntity();

	// AmmoItemComponent追加
	auto* ammo = registry.AddComponent<AmmoItemComponent>(entity);
	ammo->gridX = gridX;
	ammo->gridY = gridY;
	ammo->canPickup = false;	// 初回は回収不可
	ammo->ammoAmount = 1;

	// Transform追加
	auto* transform = registry.AddComponent<No::TransformComponent>(entity);
	transform->translate = { static_cast<float>(gridX), static_cast<float>(gridY), 0.0f };
	transform->scale = { 0.2f, 0.2f, 0.2f };

	// Mesh追加
	auto* mesh = registry.AddComponent<No::MeshComponent>(entity);
	auto* material = registry.AddComponent<No::MaterialComponent>(entity);

	NoEngine::Asset::ModelLoader::LoadModel(
		"AmmoItem",
		"resources/game/td_3105/Model/ball/ball.obj",
		mesh
	);

	material->materials = NoEngine::Asset::ModelLoader::GetMaterial("AmmoItem");
	material->color = { 1.0f, 1.0f, 0.0f, 1.0f }; // 黄色
	material->psoName = L"Renderer : Default PSO";
	material->psoId = NoEngine::Render::GetPSOID(material->psoName);
	material->rootSigId = NoEngine::Render::GetRootSignatureID(material->psoName);
}

void PlayerMovementSystem::EnableAmmoPickup(
	No::Registry& registry,
	int gridX,
	int gridY
) {
	auto view = registry.View<AmmoItemComponent>();
	if (view.Empty()) {
		return;
	}

	for (auto entity : view) {
		auto* ammo = registry.GetComponent<AmmoItemComponent>(entity);
		if (ammo->gridX == gridX && ammo->gridY == gridY) {
			ammo->canPickup = true;
		}
	}
}