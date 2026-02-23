#include "PlayerMovementSystem.h"
#include "../Component/GridCellComponent.h"
#include "../GameTag.h"
#include "../Component/PlayerBulletComponent.h"
#include "../Component/AmmoItemComponent.h"
#include "../Utility/GridUtils.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

// キーコード定義
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

		// 状態別の処理
		switch (player->state) {
		case PlayerState::OnNode:
			// HandleIntersection は OnReachNode 内でのみ呼ぶ。
			// ここで呼ぶと毎フレーム実行されてしまうため呼ばない。
			HandleNodeInput(player, registry);
			break;

		case PlayerState::MovingOnEdge:
			HandleEdgeMovement(player, deltaTime, registry);
			break;

		case PlayerState::StoppedOnEdge:
			HandleStoppedOnEdge(player, registry);
			break;
		}

		// ========== Stage2: isMoving フラグ更新 ==========
		// switch の後に更新することで、このフレームの状態変化が確定した値を使う。
		// EnemyMovementSystem はこのフラグを読んで敵の移動を制御する。
		player->isMoving = (player->state == PlayerState::MovingOnEdge);

		// Transform更新（毎フレーム必ず呼ぶことで、初期フレームも正しい位置に表示される）
		UpdateTransform(player, transform);

#ifdef USE_IMGUI
		DebugUI(player);
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
	bool inputW = NoEngine::Input::Keyboard::IsPress(KEY_W);
	bool inputS = NoEngine::Input::Keyboard::IsPress(KEY_S);
	bool inputA = NoEngine::Input::Keyboard::IsPress(KEY_A);
	bool inputD = NoEngine::Input::Keyboard::IsPress(KEY_D);

	if (!inputW && !inputS && !inputA && !inputD) {
		return;
	}

	// 初期状態（向きなし）の場合
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

	// 通常移動
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
	bool hasContinuousInput = false;

	switch (player->currentDirection) {
	case Direction::Up:    hasContinuousInput = NoEngine::Input::Keyboard::IsPress(KEY_W); break;
	case Direction::Right: hasContinuousInput = NoEngine::Input::Keyboard::IsPress(KEY_D); break;
	case Direction::Down:  hasContinuousInput = NoEngine::Input::Keyboard::IsPress(KEY_S); break;
	case Direction::Left:  hasContinuousInput = NoEngine::Input::Keyboard::IsPress(KEY_A); break;
	default: break;
	}

	// 終点近傍の自動前進チェック
	if (!hasContinuousInput && player->progressOnEdge >= NEAR_END_THRESHOLD) {
		if (HasValidNearEndInput(player, registry, player->currentDirection)) {
			hasContinuousInput = true;
		}
	}

	if (hasContinuousInput) {
		player->progressOnEdge += player->moveSpeed * deltaTime;
		UpdateRecentInputs(player, deltaTime);

		// ノード到達チェック（高速移動対応）
		while (player->progressOnEdge >= 1.0f) {
			player->progressOnEdge -= 1.0f;
			OnReachNode(player, registry);

			if (player->state != PlayerState::MovingOnEdge) {
				break;
			}
		}
	} else {
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
	if (player->actualMovingDirection == Direction::None) {
		return;
	}

	bool hasContinuousInput = false;

	switch (player->actualMovingDirection) {
	case Direction::Up:    hasContinuousInput = NoEngine::Input::Keyboard::IsPress(KEY_W); break;
	case Direction::Right: hasContinuousInput = NoEngine::Input::Keyboard::IsPress(KEY_D); break;
	case Direction::Down:  hasContinuousInput = NoEngine::Input::Keyboard::IsPress(KEY_S); break;
	case Direction::Left:  hasContinuousInput = NoEngine::Input::Keyboard::IsPress(KEY_A); break;
	default: break;
	}

	// 終点近傍の再開チェック
	if (!hasContinuousInput && player->progressOnEdge >= NEAR_END_THRESHOLD) {
		if (HasValidNearEndInput(player, registry, player->actualMovingDirection)) {
			hasContinuousInput = true;
		}
	}

	if (hasContinuousInput) {
		player->state = PlayerState::MovingOnEdge;
		player->currentDirection = player->actualMovingDirection;
	}
}

// ============================================================
//  HasValidNearEndInput
// ============================================================

bool PlayerMovementSystem::HasValidNearEndInput(
	PlayerComponent* player,
	No::Registry& registry,
	Direction futureLastDir
) {
	bool bW = NoEngine::Input::Keyboard::IsPress(KEY_W);
	bool bS = NoEngine::Input::Keyboard::IsPress(KEY_S);
	bool bA = NoEngine::Input::Keyboard::IsPress(KEY_A);
	bool bD = NoEngine::Input::Keyboard::IsPress(KEY_D);

	if (bW && CanMoveInDirection(registry,
		player->targetNodeX, player->targetNodeY, Direction::Up, futureLastDir)) {
		return true;
	}
	if (bS && CanMoveInDirection(registry,
		player->targetNodeX, player->targetNodeY, Direction::Down, futureLastDir)) {
		return true;
	}
	if (bA && CanMoveInDirection(registry,
		player->targetNodeX, player->targetNodeY, Direction::Left, futureLastDir)) {
		return true;
	}
	if (bD && CanMoveInDirection(registry,
		player->targetNodeX, player->targetNodeY, Direction::Right, futureLastDir)) {
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

	bool inputW = NoEngine::Input::Keyboard::IsPress(KEY_W);
	bool inputS = NoEngine::Input::Keyboard::IsPress(KEY_S);
	bool inputA = NoEngine::Input::Keyboard::IsPress(KEY_A);
	bool inputD = NoEngine::Input::Keyboard::IsPress(KEY_D);

	// 離されたキーを履歴から削除
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
			for (int j = i; j < player->recentInputCount - 1; ++j) {
				player->recentInputs[j] = player->recentInputs[j + 1];
			}
			player->recentInputs[player->recentInputCount - 1] = Direction::None;
			player->recentInputCount--;
		} else {
			i++;
		}
	}

	// 進行方向以外のキーを記録
	auto addInput = [&](Direction dir) {
		for (int i = 0; i < player->recentInputCount; ++i) {
			if (player->recentInputs[i] == dir) return;
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

	// 履歴ウィンドウを超えたらクリア
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
	player->currentNodeX = player->targetNodeX;
	player->currentNodeY = player->targetNodeY;
	player->state = PlayerState::OnNode;

	if (player->currentDirection != Direction::None) {
		player->lastDirection = player->currentDirection;
	}

	HandleIntersection(player, registry);

	Direction nextDir = Direction::None;

	// 1. 入力履歴から最新の入力を優先的にチェック（逆順＝最新優先）
	for (int i = player->recentInputCount - 1; i >= 0; --i) {
		Direction dir = player->recentInputs[i];
		if (CanMoveInDirection(registry, player->currentNodeX,
			player->currentNodeY, dir, player->lastDirection)) {
			nextDir = dir;
			break;
		}
	}

	// 2. 履歴になければ現在の入力をチェック（曲がり優先）
	if (nextDir == Direction::None) {
		bool inputW = NoEngine::Input::Keyboard::IsPress(KEY_W);
		bool inputS = NoEngine::Input::Keyboard::IsPress(KEY_S);
		bool inputA = NoEngine::Input::Keyboard::IsPress(KEY_A);
		bool inputD = NoEngine::Input::Keyboard::IsPress(KEY_D);

		if (player->currentDirection == Direction::Right) {
			if (inputW && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Up, player->lastDirection))
				nextDir = Direction::Up;
			else if (inputS && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Down, player->lastDirection))
				nextDir = Direction::Down;
			else if (inputD && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Right, player->lastDirection))
				nextDir = Direction::Right;
		} else if (player->currentDirection == Direction::Left) {
			if (inputW && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Up, player->lastDirection))
				nextDir = Direction::Up;
			else if (inputS && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Down, player->lastDirection))
				nextDir = Direction::Down;
			else if (inputA && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Left, player->lastDirection))
				nextDir = Direction::Left;
		} else if (player->currentDirection == Direction::Up) {
			if (inputA && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Left, player->lastDirection))
				nextDir = Direction::Left;
			else if (inputD && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Right, player->lastDirection))
				nextDir = Direction::Right;
			else if (inputW && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Up, player->lastDirection))
				nextDir = Direction::Up;
		} else if (player->currentDirection == Direction::Down) {
			if (inputA && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Left, player->lastDirection))
				nextDir = Direction::Left;
			else if (inputD && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Right, player->lastDirection))
				nextDir = Direction::Right;
			else if (inputS && CanMoveInDirection(registry, player->currentNodeX, player->currentNodeY, Direction::Down, player->lastDirection))
				nextDir = Direction::Down;
		}
	}

	// 入力履歴をクリア
	player->recentInputCount = 0;
	player->inputHistoryTime = 0.0f;
	for (int i = 0; i < 4; ++i) {
		player->recentInputs[i] = Direction::None;
	}

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
	(void)registry;

	if (player->lastDirection == Direction::None) {
		player->lastDirection = dir;
	}

	player->actualMovingDirection = dir;

	GridUtils::GetNextNodeCoords(
		player->currentNodeX,
		player->currentNodeY,
		dir,
		player->targetNodeX,
		player->targetNodeY
	);

	player->currentDirection = dir;
	player->state = PlayerState::MovingOnEdge;
	player->progressOnEdge = 0.0f;
	player->isAtDeadEnd = false;
}

// ============================================================
//  StopMovement
// ============================================================

void PlayerMovementSystem::StopMovement(PlayerComponent* player) {
	player->currentDirection = Direction::None;
	player->state = PlayerState::StoppedOnEdge;
}

// ============================================================
//  CanMoveInDirection
//  ゲーム固有ロジック（後退禁止＋行き止まり例外）を含むため GridUtils に移さない。
//  Stage3 で EnemyMovementSystem でも同じロジックを使用する予定。
// ============================================================

bool PlayerMovementSystem::CanMoveInDirection(
	No::Registry& registry,
	int nodeX, int nodeY,
	Direction dir,
	Direction lastDir
) {
	auto* cell = GridUtils::GetGridCell(registry, nodeX, nodeY);
	if (!cell) return false;

	if (!GridUtils::HasConnection(cell, dir)) return false;

	// 初期状態なら後退判定スキップ
	if (lastDir == Direction::None) return true;

	Direction opposite = GridUtils::GetOppositeDirection(lastDir);

	// 後退判定
	if (dir == opposite) {
		// 行き止まりなら例外的に後退OK
		bool forwardBlocked = !GridUtils::HasConnection(cell, lastDir);
		bool onlyBackward = GridUtils::HasConnection(cell, opposite);
		int  connectionCount = GridUtils::CountConnections(cell);

		if (forwardBlocked && onlyBackward && connectionCount == 1) {
			return true; // 行き止まり例外
		}
		return false; // 通常の後退は不可
	}

	return true;
}

// ============================================================
//  CalculateWorldPosition
//  GridUtils::GridToWorld を使用しているため、
//  スケール変更時はここではなく GridUtils.h の scale デフォルト値を変える。
// ============================================================

No::Vector3 PlayerMovementSystem::CalculateWorldPosition(
	const PlayerComponent* player
) {
	if (player->state == PlayerState::OnNode) {
		return GridUtils::GridToWorld(player->currentNodeX, player->currentNodeY);
	} else {
		// エッジ上を線形補間（MovingOnEdge or StoppedOnEdge）
		No::Vector3 currentPos = GridUtils::GridToWorld(player->currentNodeX, player->currentNodeY);
		No::Vector3 targetPos = GridUtils::GridToWorld(player->targetNodeX, player->targetNodeY);
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
	transform->translate = CalculateWorldPosition(player);
	transform->rotation = CalcDirectionRotation(player->actualMovingDirection);
}

// ============================================================
//  CalcDirectionRotation
// ============================================================

NoEngine::Math::Quaternion PlayerMovementSystem::CalcDirectionRotation(Direction dir) {
	NoEngine::Math::Quaternion q;
	switch (dir) {
	case Direction::None:
		q.FromAxisAngle(NoEngine::Math::Vector3{ 0.0f, 1.0f, 0.0f }, PI);
		break;
	case Direction::Up:
		q.FromAxisAngle(NoEngine::Math::Vector3{ 1.0f, 0.0f, 0.0f }, -PI * 0.5f);
		break;
	case Direction::Down:
		q.FromAxisAngle(NoEngine::Math::Vector3{ 1.0f, 0.0f, 0.0f }, PI * 0.5f);
		break;
	case Direction::Right:
		q.FromAxisAngle(NoEngine::Math::Vector3{ 0.0f, 1.0f, 0.0f }, PI * 0.5f);
		break;
	case Direction::Left:
		q.FromAxisAngle(NoEngine::Math::Vector3{ 0.0f, 1.0f, 0.0f }, -PI * 0.5f);
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
	auto* cell = GridUtils::GetGridCell(registry, player->currentNodeX, player->currentNodeY);
	if (!cell) return;

	int connectionCount = GridUtils::CountConnections(cell);
	Direction opposite = GridUtils::GetOppositeDirection(player->lastDirection);
	bool onlyBackward = (connectionCount == 1) && GridUtils::HasConnection(cell, opposite);

	player->isAtDeadEnd = onlyBackward;
}

// ============================================================
//  デバッグ UI
// ============================================================

#ifdef USE_IMGUI
void PlayerMovementSystem::DebugUI(PlayerComponent* player) {
	ImGui::Begin("Player Movement");

	ImGui::Text("=== Position ===");
	ImGui::Text("Current Node: (%d, %d)", player->currentNodeX, player->currentNodeY);

	if (player->state != PlayerState::OnNode) {
		ImGui::Text("Target Node: (%d, %d)", player->targetNodeX, player->targetNodeY);
		ImGui::ProgressBar(player->progressOnEdge, ImVec2(-1, 0), "Progress");
	}

	ImGui::Separator();
	ImGui::Text("=== State ===");
	ImGui::Text("State: %s", StateToString(player->state));
	ImGui::Text("isMoving: %s", player->isMoving ? "true" : "false"); // Stage2確認用
	ImGui::Text("At Dead End: %s", player->isAtDeadEnd ? "Yes" : "No");

	ImGui::Separator();
	ImGui::Text("=== Direction ===");
	ImGui::Text("Current: %s", DirectionToString(player->currentDirection));
	ImGui::Text("Last: %s", DirectionToString(player->lastDirection));
	ImGui::Text("Actual Moving: %s", DirectionToString(player->actualMovingDirection));

	ImGui::Separator();
	ImGui::Text("=== Input History ===");
	ImGui::Text("Count: %d", player->recentInputCount);
	for (int i = 0; i < player->recentInputCount; ++i) {
		ImGui::Text("  [%d] %s", i, DirectionToString(player->recentInputs[i]));
	}
	ImGui::Text("History Time: %.2f", player->inputHistoryTime);

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
//  交差点検出と弾薬配置
// ============================================================

bool PlayerMovementSystem::IsIntersection(const GridCellComponent* cell) {
	if (!cell) return false;
	return GridUtils::CountConnections(cell) >= 3;
}

void PlayerMovementSystem::HandleIntersection(
	PlayerComponent* player,
	No::Registry& registry
) {
	auto* cell = GridUtils::GetGridCell(registry, player->currentNodeX, player->currentNodeY);

	if (!IsIntersection(cell)) return;

	if (!HasAmmoAtPosition(registry, player->currentNodeX, player->currentNodeY)) {
		CreateAmmoItem(registry, player->currentNodeX, player->currentNodeY);
	} else {
		EnableAmmoPickup(registry, player->currentNodeX, player->currentNodeY);
	}
}

bool PlayerMovementSystem::HasAmmoAtPosition(
	No::Registry& registry,
	int gridX,
	int gridY
) {
	auto view = registry.View<AmmoItemComponent>();
	if (view.Empty()) return false;

	for (auto entity : view) {
		auto* ammo = registry.GetComponent<AmmoItemComponent>(entity);
		if (ammo->gridX == gridX && ammo->gridY == gridY) return true;
	}
	return false;
}

void PlayerMovementSystem::CreateAmmoItem(
	No::Registry& registry,
	int gridX,
	int gridY
) {
	auto entity = registry.GenerateEntity();

	registry.AddComponent<AmmoItemTag>(entity);
	registry.AddComponent<DeathFlag>(entity);

	auto* ammo = registry.AddComponent<AmmoItemComponent>(entity);
	ammo->gridX = gridX;
	ammo->gridY = gridY;
	ammo->canPickup = false;
	ammo->ammoAmount = 1;

	auto* transform = registry.AddComponent<No::TransformComponent>(entity);
	transform->translate = GridUtils::GridToWorld(gridX, gridY);
	transform->scale = { 0.2f, 0.2f, 0.2f };

	auto* mesh = registry.AddComponent<No::MeshComponent>(entity);
	auto* material = registry.AddComponent<No::MaterialComponent>(entity);

	NoEngine::Asset::ModelLoader::LoadModel(
		"AmmoItem",
		"resources/game/td_3105/Model/ball/ball.obj",
		mesh
	);

	material->materials = NoEngine::Asset::ModelLoader::GetMaterial("AmmoItem");
	material->color = { 1.0f, 1.0f, 0.0f, 1.0f };
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
	if (view.Empty()) return;

	for (auto entity : view) {
		auto* ammo = registry.GetComponent<AmmoItemComponent>(entity);
		if (ammo->gridX == gridX && ammo->gridY == gridY) {
			ammo->canPickup = true;
		}
	}
}