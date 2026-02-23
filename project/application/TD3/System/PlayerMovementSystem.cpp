#include "PlayerMovementSystem.h"
#include "../Component/GridCellComponent.h"
#include "../Component/PlayerTag.h"

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

// ============================================================
//  Update
// ============================================================

void PlayerMovementSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<PlayerComponent, PlayerTag, No::TransformComponent>();

	for (auto entity : view) {
		auto* player = registry.GetComponent<PlayerComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);

		// 状態別の処理を振り分ける
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

		// 位置・回転を Transform に反映する
		UpdateTransform(player, transform);

#ifdef USE_IMGUI
		ShowPlayerDebugUI(player);
#endif
	}
}

// ============================================================
//  ノード上の入力処理
// ============================================================

void PlayerMovementSystem::HandleNodeInput(
	PlayerComponent* player,
	No::Registry& registry
) {
	bool inputW = NoEngine::Input::Keyboard::IsPress(KEY_W);
	bool inputS = NoEngine::Input::Keyboard::IsPress(KEY_S);
	bool inputA = NoEngine::Input::Keyboard::IsPress(KEY_A);
	bool inputD = NoEngine::Input::Keyboard::IsPress(KEY_D);

	// 何も押されていなければ何もしない
	if (!inputW && !inputS && !inputA && !inputD) {
		return;
	}

	// 初回（lastDirection が未設定）は後退判定なしで移動開始
	if (player->lastDirection == Direction::None) {
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

	// 通常：接続があり後退でない方向に移動
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
//  エッジ移動中の処理
// ============================================================

void PlayerMovementSystem::HandleEdgeMovement(
	PlayerComponent* player,
	float deltaTime,
	No::Registry& registry
) {
	// ---- 現在の進行方向キーが押されているか ----
	bool hasContinuousInput = false;
	switch (player->currentDirection) {
	case Direction::Up:    hasContinuousInput = NoEngine::Input::Keyboard::IsPress(KEY_W); break;
	case Direction::Right: hasContinuousInput = NoEngine::Input::Keyboard::IsPress(KEY_D); break;
	case Direction::Down:  hasContinuousInput = NoEngine::Input::Keyboard::IsPress(KEY_S); break;
	case Direction::Left:  hasContinuousInput = NoEngine::Input::Keyboard::IsPress(KEY_A); break;
	default: break;
	}

	// ---- 終点近傍の先行入力チェック（NEAR_END_THRESHOLD 以上のとき） ----
	// ターゲットノードで有効な方向キーを押しているなら、
	// 現在方向キーなしでも自動前進を許可し、その方向を bufferedDirection に記録する。
	if (player->progressOnEdge >= NEAR_END_THRESHOLD) {
		Direction nearEndDir = GetNearEndInputDirection(player, registry);

		if (nearEndDir != Direction::None) {
			// 自動前進フラグを立てる
			hasContinuousInput = true;
			// ターゲットノード到達時に使う方向をバッファに保存
			// （すでに同じ方向が入っている場合は上書きしない）
			if (player->bufferedDirection == Direction::None) {
				player->bufferedDirection = nearEndDir;
			}
		}
	}

	if (hasContinuousInput) {
		// 前進
		player->progressOnEdge += player->moveSpeed * deltaTime;

		// 進行方向以外のキー入力を履歴に記録する
		UpdateRecentInputs(player, deltaTime);

		// ノード到達チェック（高速移動で複数ノードを跨ぐ場合のループ対応）
		while (player->progressOnEdge >= 1.0f) {
			player->progressOnEdge -= 1.0f;
			OnReachNode(player, registry);

			// OnReachNode 内で停止状態になった場合はループを抜ける
			if (player->state != PlayerState::MovingOnEdge) {
				break;
			}
		}
	} else {
		// 入力なし → エッジ途中で停止（progressOnEdge は保持）
		// 停止するのでバッファをクリアしておく
		player->bufferedDirection = Direction::None;
		StopMovement(player);
	}
}

// ============================================================
//  エッジ途中で停止中の処理
// ============================================================

void PlayerMovementSystem::HandleStoppedOnEdge(
	PlayerComponent* player,
	No::Registry& registry
) {
	// actualMovingDirection が未設定なら何もしない（初期状態ガード）
	if (player->actualMovingDirection == Direction::None) {
		return;
	}

	// ---- 来た方向のキーで移動再開 ----
	bool hasContinuousInput = false;
	switch (player->actualMovingDirection) {
	case Direction::Up:    hasContinuousInput = NoEngine::Input::Keyboard::IsPress(KEY_W); break;
	case Direction::Right: hasContinuousInput = NoEngine::Input::Keyboard::IsPress(KEY_D); break;
	case Direction::Down:  hasContinuousInput = NoEngine::Input::Keyboard::IsPress(KEY_S); break;
	case Direction::Left:  hasContinuousInput = NoEngine::Input::Keyboard::IsPress(KEY_A); break;
	default: break;
	}

	// ---- 終点近傍の先行入力チェック（NEAR_END_THRESHOLD 以上のとき） ----
	// 停止中でも終点に近ければ、ターゲットノードで有効な方向キーで再開を許可する
	if (!hasContinuousInput && player->progressOnEdge >= NEAR_END_THRESHOLD) {
		Direction nearEndDir = GetNearEndInputDirection(player, registry);

		if (nearEndDir != Direction::None) {
			// 再開フラグを立てる
			hasContinuousInput = true;
			// バッファに記録（次のノード到達時に使用）
			if (player->bufferedDirection == Direction::None) {
				player->bufferedDirection = nearEndDir;
			}
		}
	}

	if (hasContinuousInput) {
		// 移動再開（progressOnEdge はそのまま、同じ方向で継続）
		player->state = PlayerState::MovingOnEdge;
		player->currentDirection = player->actualMovingDirection;
	}
	// それ以外の入力（有効でない方向キーなど）は無視する
}

// ============================================================
//  終点近傍での有効方向キー取得
// ============================================================

Direction PlayerMovementSystem::GetNearEndInputDirection(
	PlayerComponent* player,
	No::Registry& registry
) {
	bool bW = NoEngine::Input::Keyboard::IsPress(KEY_W);
	bool bS = NoEngine::Input::Keyboard::IsPress(KEY_S);
	bool bA = NoEngine::Input::Keyboard::IsPress(KEY_A);
	bool bD = NoEngine::Input::Keyboard::IsPress(KEY_D);

	// ターゲットノードに到達した後の lastDirection = 現在の進行方向
	Direction futureLastDir = player->currentDirection;

	// ---- 曲がり方向（進行方向以外）を優先してチェック ----
	// 例：上に進んでいるとき、左右キーを先に確認する
	struct KeyCheck { Direction dir; bool pressed; };
	KeyCheck checks[4] = {
		{ Direction::Up,    bW },
		{ Direction::Down,  bS },
		{ Direction::Left,  bA },
		{ Direction::Right, bD },
	};

	// パス1：進行方向以外（曲がり方向）を優先
	for (auto& c : checks) {
		if (c.dir == player->currentDirection) continue;  // 直進は後回し
		if (c.pressed && CanMoveInDirection(registry,
			player->targetNodeX, player->targetNodeY,
			c.dir, futureLastDir)) {
			return c.dir;
		}
	}

	// パス2：直進チェック
	for (auto& c : checks) {
		if (c.dir != player->currentDirection) continue;  // 曲がり方向はスキップ
		if (c.pressed && CanMoveInDirection(registry,
			player->targetNodeX, player->targetNodeY,
			c.dir, futureLastDir)) {
			return c.dir;
		}
	}

	return Direction::None;
}

// ============================================================
//  入力履歴の更新
// ============================================================

void PlayerMovementSystem::UpdateRecentInputs(
	PlayerComponent* player,
	float deltaTime
) {
	player->inputHistoryTime += deltaTime;

	bool inputW = NoEngine::Input::Keyboard::IsPress(KEY_W);
	bool inputS = NoEngine::Input::Keyboard::IsPress(KEY_S);
	bool inputA = NoEngine::Input::Keyboard::IsPress(KEY_A);
	bool inputD = NoEngine::Input::Keyboard::IsPress(KEY_D);

	// ---- 離されたキーを履歴から削除（配列を前に詰める） ----
	for (int i = 0; i < player->recentInputCount; ) {
		Direction dir = player->recentInputs[i];
		bool stillPressed = false;

		switch (dir) {
		case Direction::Up:    stillPressed = inputW; break;
		case Direction::Down:  stillPressed = inputS; break;
		case Direction::Left:  stillPressed = inputA; break;
		case Direction::Right: stillPressed = inputD; break;
		default: break;
		}

		if (!stillPressed) {
			// 削除：後ろを前に詰める
			for (int j = i; j < player->recentInputCount - 1; ++j) {
				player->recentInputs[j] = player->recentInputs[j + 1];
			}
			player->recentInputs[player->recentInputCount - 1] = Direction::None;
			player->recentInputCount--;
			// i はインクリメントしない（詰めた次の要素を確認するため）
		} else {
			i++;
		}
	}

	// ---- 進行方向以外の押下キーを末尾に追加（重複は無視） ----
	auto addInput = [&](Direction dir) {
		for (int i = 0; i < player->recentInputCount; ++i) {
			if (player->recentInputs[i] == dir) return;  // 既に記録済み
		}
		if (player->recentInputCount < 4) {
			player->recentInputs[player->recentInputCount] = dir;
			player->recentInputCount++;
		}
		};

	if (inputW && player->currentDirection != Direction::Up)    addInput(Direction::Up);
	if (inputS && player->currentDirection != Direction::Down)  addInput(Direction::Down);
	if (inputA && player->currentDirection != Direction::Left)  addInput(Direction::Left);
	if (inputD && player->currentDirection != Direction::Right) addInput(Direction::Right);

	// ---- 保持時間を超えたら履歴をクリア ----
	if (player->inputHistoryTime > player->inputHistoryWindow) {
		player->recentInputCount = 0;
		player->inputHistoryTime = 0.0f;
		for (int i = 0; i < 4; ++i) {
			player->recentInputs[i] = Direction::None;
		}
	}
}

// ============================================================
//  ノード到達処理
// ============================================================

void PlayerMovementSystem::OnReachNode(
	PlayerComponent* player,
	No::Registry& registry
) {
	// 現在座標をターゲットノードに更新
	player->currentNodeX = player->targetNodeX;
	player->currentNodeY = player->targetNodeY;
	player->state = PlayerState::OnNode;

	// 来た方向を記録（後退判定・次の移動に使用）
	if (player->currentDirection != Direction::None) {
		player->lastDirection = player->currentDirection;
	}

	Direction nextDir = Direction::None;

	// ========== 優先度 0：終点近傍バッファ ==========
	// HandleEdgeMovement / HandleStoppedOnEdge で記録した方向を最優先で使う
	if (player->bufferedDirection != Direction::None) {
		Direction buffered = player->bufferedDirection;
		player->bufferedDirection = Direction::None;  // 消費してクリア

		if (CanMoveInDirection(registry,
			player->currentNodeX, player->currentNodeY,
			buffered, player->lastDirection)) {
			nextDir = buffered;
		}
		// バッファが無効だった場合は次の優先度に落ちる
	}

	// ========== 優先度 1：入力履歴（最新順） ==========
	if (nextDir == Direction::None) {
		for (int i = player->recentInputCount - 1; i >= 0; --i) {
			Direction dir = player->recentInputs[i];
			if (CanMoveInDirection(registry,
				player->currentNodeX, player->currentNodeY,
				dir, player->lastDirection)) {
				nextDir = dir;
				break;
			}
		}
	}

	// ========== 優先度 2：現在の入力（進行方向別に曲がり優先） ==========
	if (nextDir == Direction::None) {
		bool inputW = NoEngine::Input::Keyboard::IsPress(KEY_W);
		bool inputS = NoEngine::Input::Keyboard::IsPress(KEY_S);
		bool inputA = NoEngine::Input::Keyboard::IsPress(KEY_A);
		bool inputD = NoEngine::Input::Keyboard::IsPress(KEY_D);

		if (player->currentDirection == Direction::Right) {
			// 右進行：上下（曲がり）優先 → 直進
			if (inputW && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Up, player->lastDirection)) nextDir = Direction::Up;
			else if (inputS && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Down, player->lastDirection)) nextDir = Direction::Down;
			else if (inputD && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Right, player->lastDirection)) nextDir = Direction::Right;
		} else if (player->currentDirection == Direction::Left) {
			// 左進行：上下（曲がり）優先 → 直進
			if (inputW && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Up, player->lastDirection)) nextDir = Direction::Up;
			else if (inputS && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Down, player->lastDirection)) nextDir = Direction::Down;
			else if (inputA && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Left, player->lastDirection)) nextDir = Direction::Left;
		} else if (player->currentDirection == Direction::Up) {
			// 上進行：左右（曲がり）優先 → 直進
			if (inputA && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Left, player->lastDirection)) nextDir = Direction::Left;
			else if (inputD && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Right, player->lastDirection)) nextDir = Direction::Right;
			else if (inputW && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Up, player->lastDirection)) nextDir = Direction::Up;
		} else if (player->currentDirection == Direction::Down) {
			// 下進行：左右（曲がり）優先 → 直進
			if (inputA && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Left, player->lastDirection)) nextDir = Direction::Left;
			else if (inputD && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Right, player->lastDirection)) nextDir = Direction::Right;
			else if (inputS && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Down, player->lastDirection)) nextDir = Direction::Down;
		}
	}

	// 入力履歴をクリア（次のノード間で使い回さない）
	player->recentInputCount = 0;
	player->inputHistoryTime = 0.0f;
	for (int i = 0; i < 4; ++i) {
		player->recentInputs[i] = Direction::None;
	}

	// 次方向が決まれば移動開始、決まらなければノードで停止
	if (nextDir != Direction::None) {
		StartMovement(player, nextDir, registry);
	} else {
		player->currentDirection = Direction::None;
		CheckDeadEnd(player, registry);
	}
}

// ============================================================
//  移動開始
// ============================================================

void PlayerMovementSystem::StartMovement(
	PlayerComponent* player,
	Direction dir,
	No::Registry& registry
) {
	(void)registry;

	// 初回移動のみ lastDirection を設定（以降は OnReachNode で更新）
	if (player->lastDirection == Direction::None) {
		player->lastDirection = dir;
	}

	// 実際に移動した方向を記録（停止再開・モデル向きに使用）
	player->actualMovingDirection = dir;

	// ターゲットノード座標を計算
	GetNextNodeCoords(
		player->currentNodeX, player->currentNodeY,
		dir,
		player->targetNodeX, player->targetNodeY
	);

	player->currentDirection = dir;
	player->state = PlayerState::MovingOnEdge;
	player->progressOnEdge = 0.0f;
	player->isAtDeadEnd = false;
}

// ============================================================
//  移動停止
// ============================================================

void PlayerMovementSystem::StopMovement(PlayerComponent* player) {
	// progressOnEdge はそのまま保持してエッジ上で止まる
	player->currentDirection = Direction::None;
	player->state = PlayerState::StoppedOnEdge;
}

// ============================================================
//  移動可能判定
// ============================================================

bool PlayerMovementSystem::CanMoveInDirection(
	No::Registry& registry,
	int nodeX, int nodeY,
	Direction dir,
	Direction lastDir
) {
	auto* cell = GetGridCell(registry, nodeX, nodeY);
	if (!cell) return false;

	// そもそも接続がなければ不可
	if (!HasConnection(cell, dir)) return false;

	// 初期状態（lastDir 未設定）は後退判定をスキップ
	if (lastDir == Direction::None) return true;

	Direction opposite = GetOppositeDirection(lastDir);

	// 後退方向への移動は原則禁止
	if (dir == opposite) {
		// 例外：行き止まり（接続が来た方向 1 本のみ）なら後退を許可
		int connectionCount = 0;
		if (cell->hasConnectionUp)    connectionCount++;
		if (cell->hasConnectionRight) connectionCount++;
		if (cell->hasConnectionDown)  connectionCount++;
		if (cell->hasConnectionLeft)  connectionCount++;

		bool forwardBlocked = !HasConnection(cell, lastDir);
		bool hasBackward = HasConnection(cell, opposite);

		if (forwardBlocked && hasBackward && connectionCount == 1) {
			return true;  // 行き止まり例外：後退 OK
		}
		return false;
	}

	return true;
}

// ============================================================
//  グリッドセル取得
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
//  接続チェック
// ============================================================

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

// ============================================================
//  反対方向取得
// ============================================================

Direction PlayerMovementSystem::GetOppositeDirection(Direction dir) {
	switch (dir) {
	case Direction::Up:    return Direction::Down;
	case Direction::Right: return Direction::Left;
	case Direction::Down:  return Direction::Up;
	case Direction::Left:  return Direction::Right;
	default:               return Direction::None;
	}
}

// ============================================================
//  次ノード座標計算
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
//  ワールド座標計算
// ============================================================

No::Vector3 PlayerMovementSystem::CalculateWorldPosition(
	const PlayerComponent* player
) {
	if (player->state == PlayerState::OnNode) {
		// ノード上：そのままノード座標
		return No::Vector3{
			static_cast<float>(player->currentNodeX),
			static_cast<float>(player->currentNodeY),
			0.0f
		};
	} else {
		// エッジ上：現在ノードとターゲットノードを progress で線形補間
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
//  Transform 更新
// ============================================================

void PlayerMovementSystem::UpdateTransform(
	PlayerComponent* player,
	No::TransformComponent* transform
) {
	// 位置を更新する
	transform->translate = CalculateWorldPosition(player);

	// 回転を更新する
	// actualMovingDirection を使うことで、停止中も最後に動いた方向を向き続ける
	transform->rotation = CalcDirectionRotation(player->actualMovingDirection);
}

// ============================================================
//  移動方向 → 回転クオータニオン
// ============================================================

NoEngine::Math::Quaternion PlayerMovementSystem::CalcDirectionRotation(Direction dir) {
	// ---- DirectX 左手座標系での回転ロジック ----
	//
	// 前提：モデルは identity 回転のとき Z+ 方向（画面奥）を向いている
	//
	// DirectX 左手座標系の Y 軸回転（+θ = Y 軸を上から見て時計回り）：
	//   Z+ を +90° 回転 → X+ 方向（Right）
	//   Z+ を -90° 回転 → X- 方向（Left）
	//   Z+ を +180° 回転 → Z- 方向（カメラ側・初期向き）
	//
	// DirectX 左手座標系の X 軸回転（+θ = X 軸の正方向から見て時計回り）：
	//   Z+ を +90° 回転 → Y+ 方向（Up）
	//   Z+ を -90° 回転 → Y- 方向（Down）
	//

	NoEngine::Math::Quaternion q;
	switch (dir) {
	case Direction::None:
		// 初期向き：Z- を向く（カメラ側） = Y 軸 +180°
		q.FromAxisAngle(
			NoEngine::Math::Vector3{ 0.0f, 1.0f, 0.0f }, PI);
		break;

	case Direction::Up:
		// Y+ 方向を向く = X 軸 +90°
		q.FromAxisAngle(
			NoEngine::Math::Vector3{ 1.0f, 0.0f, 0.0f }, PI * 0.5f);
		break;

	case Direction::Down:
		// Y- 方向を向く = X 軸 -90°
		q.FromAxisAngle(
			NoEngine::Math::Vector3{ 1.0f, 0.0f, 0.0f }, -PI * 0.5f);
		break;

	case Direction::Right:
		// X+ 方向を向く = Y 軸 +90°
		q.FromAxisAngle(
			NoEngine::Math::Vector3{ 0.0f, 1.0f, 0.0f }, PI * 0.5f);
		break;

	case Direction::Left:
		// X- 方向を向く = Y 軸 -90°
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
//  行き止まりチェック
// ============================================================

void PlayerMovementSystem::CheckDeadEnd(
	PlayerComponent* player,
	No::Registry& registry
) {
	auto* cell = GetGridCell(registry, player->currentNodeX, player->currentNodeY);
	if (!cell) return;

	int connectionCount = 0;
	if (cell->hasConnectionUp)    connectionCount++;
	if (cell->hasConnectionRight) connectionCount++;
	if (cell->hasConnectionDown)  connectionCount++;
	if (cell->hasConnectionLeft)  connectionCount++;

	// 接続が 1 本かつ来た方向のみ = 行き止まり
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
	ImGui::Text("Current Node: (%d, %d)", player->currentNodeX, player->currentNodeY);

	if (player->state != PlayerState::OnNode) {
		ImGui::Text("Target Node: (%d, %d)", player->targetNodeX, player->targetNodeY);
		ImGui::ProgressBar(player->progressOnEdge, ImVec2(-1, 0), "Progress");
	}

	// 状態
	ImGui::Separator();
	ImGui::Text("=== State ===");
	ImGui::Text("State: %s", StateToString(player->state));
	ImGui::Text("At Dead End: %s", player->isAtDeadEnd ? "Yes" : "No");

	// 方向
	ImGui::Separator();
	ImGui::Text("=== Direction ===");
	ImGui::Text("Current:      %s", DirectionToString(player->currentDirection));
	ImGui::Text("Last:         %s", DirectionToString(player->lastDirection));
	ImGui::Text("Actual Moving:%s", DirectionToString(player->actualMovingDirection));
	ImGui::Text("Buffered:     %s", DirectionToString(player->bufferedDirection));

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