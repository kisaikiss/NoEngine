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

		// ========== isMoving フラグ更新 ==========
		// switch の後（状態確定後）に更新することで、このフレームの状態変化が確定した値を使う。
		// EnemyMovementSystem はこのフラグを参照し、プレイヤーが動いているときだけ敵を動かす。
		player->isMoving = (player->state == PlayerState::MovingOnEdge);

		// Transform 更新
		// 毎フレーム必ず呼ぶことで、初期フレームも正しい位置に表示される。
		// （InitializePlayer で transform.translate を手動設定しなくてよい理由）
		UpdateTransform(player, transform);

		// デバッグUI
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
		// GridUtils::CanMoveInDirection で後退禁止 + 行き止まり例外の共通ロジックを使用
		if (inputW && GridUtils::CanMoveInDirection(registry, player->currentNodeX,
			player->currentNodeY, Direction::Up, Direction::None)) {
			StartMovement(player, Direction::Up, registry);
		} else if (inputD && GridUtils::CanMoveInDirection(registry, player->currentNodeX,
			player->currentNodeY, Direction::Right, Direction::None)) {
			StartMovement(player, Direction::Right, registry);
		} else if (inputS && GridUtils::CanMoveInDirection(registry, player->currentNodeX,
			player->currentNodeY, Direction::Down, Direction::None)) {
			StartMovement(player, Direction::Down, registry);
		} else if (inputA && GridUtils::CanMoveInDirection(registry, player->currentNodeX,
			player->currentNodeY, Direction::Left, Direction::None)) {
			StartMovement(player, Direction::Left, registry);
		}
		return;
	}

	// 通常移動（lastDirection がある状態）
	if (inputW && GridUtils::CanMoveInDirection(registry, player->currentNodeX,
		player->currentNodeY, Direction::Up, player->lastDirection)) {
		StartMovement(player, Direction::Up, registry);
	} else if (inputD && GridUtils::CanMoveInDirection(registry, player->currentNodeX,
		player->currentNodeY, Direction::Right, player->lastDirection)) {
		StartMovement(player, Direction::Right, registry);
	} else if (inputS && GridUtils::CanMoveInDirection(registry, player->currentNodeX,
		player->currentNodeY, Direction::Down, player->lastDirection)) {
		StartMovement(player, Direction::Down, registry);
	} else if (inputA && GridUtils::CanMoveInDirection(registry, player->currentNodeX,
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
	// ---- 現在の進行方向キーが押されているか ----
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

	// 終点近傍の自動前進チェック
	if (!hasContinuousInput && player->progressOnEdge >= NEAR_END_THRESHOLD) {
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
		// 入力なし：エッジ途中で停止（progressOnEdge 保持）
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

	// 終点近傍の再開チェック
	if (!hasContinuousInput && player->progressOnEdge >= NEAR_END_THRESHOLD) {
		if (HasValidNearEndInput(player, registry, player->actualMovingDirection)) {
			hasContinuousInput = true;
		}
	}

	if (hasContinuousInput) {
		// 移動再開（progressOnEdge 保持、同じ方向に継続）
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

	// 各キーについて「ターゲットノードで有効か」を GridUtils::CanMoveInDirection で判定する
	// ※方向は返さない。どの方向に進むかは OnReachNode の recentInputs に任せる
	if (bW && GridUtils::CanMoveInDirection(registry,
		player->targetNodeX, player->targetNodeY,
		Direction::Up, futureLastDir)) {
		return true;
	}
	if (bS && GridUtils::CanMoveInDirection(registry,
		player->targetNodeX, player->targetNodeY,
		Direction::Down, futureLastDir)) {
		return true;
	}
	if (bA && GridUtils::CanMoveInDirection(registry,
		player->targetNodeX, player->targetNodeY,
		Direction::Left, futureLastDir)) {
		return true;
	}
	if (bD && GridUtils::CanMoveInDirection(registry,
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

		switch (dir) {
		case Direction::Up:    stillPressed = inputW; break;
		case Direction::Down:  stillPressed = inputS; break;
		case Direction::Left:  stillPressed = inputA; break;
		case Direction::Right: stillPressed = inputD; break;
		default: break;
		}

		if (!stillPressed) {
			// 離されたキーを履歴から削除（配列を詰める）
			for (int j = i; j < player->recentInputCount - 1; ++j) {
				player->recentInputs[j] = player->recentInputs[j + 1];
			}
			player->recentInputs[player->recentInputCount - 1] = Direction::None;
			player->recentInputCount--;
		} else {
			i++;
		}
	}

	// ========== 進行方向以外のキーを記録 ==========
	auto addInput = [&](Direction dir) {
		for (int i = 0; i < player->recentInputCount; ++i) {
			if (player->recentInputs[i] == dir) return; // 既に記録済み
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
	if (player->currentDirection != Direction::None) {
		player->lastDirection = player->currentDirection;
	}

	// 交差点検出（弾薬の配置・回収可能化）
	HandleIntersection(player, registry);

	Direction nextDir = Direction::None;

	// ========== 1. 入力履歴から最新の入力を優先的にチェック ==========
	// 逆順（最新から）でチェックすることで最後に押したキーが優先される
	for (int i = player->recentInputCount - 1; i >= 0; --i) {
		Direction dir = player->recentInputs[i];

		if (GridUtils::CanMoveInDirection(registry, player->currentNodeX,
			player->currentNodeY, dir, player->lastDirection)) {
			nextDir = dir;
			break;
		}
	}

	// ========== 2. 履歴になければ現在の入力をチェック ==========
	if (nextDir == Direction::None) {
		bool inputW = NoEngine::Input::Keyboard::IsPress(KEY_W);
		bool inputS = NoEngine::Input::Keyboard::IsPress(KEY_S);
		bool inputA = NoEngine::Input::Keyboard::IsPress(KEY_A);
		bool inputD = NoEngine::Input::Keyboard::IsPress(KEY_D);

		// 進行方向別に曲がり優先判定
		if (player->currentDirection == Direction::Right) {
			// 右進行中 → 上下優先、直進次点
			if (inputW && GridUtils::CanMoveInDirection(registry, player->currentNodeX,
				player->currentNodeY, Direction::Up, player->lastDirection)) {
				nextDir = Direction::Up;
			} else if (inputS && GridUtils::CanMoveInDirection(registry, player->currentNodeX,
				player->currentNodeY, Direction::Down, player->lastDirection)) {
				nextDir = Direction::Down;
			} else if (inputD && GridUtils::CanMoveInDirection(registry, player->currentNodeX,
				player->currentNodeY, Direction::Right, player->lastDirection)) {
				nextDir = Direction::Right;
			}
		} else if (player->currentDirection == Direction::Left) {
			// 左進行中 → 上下優先
			if (inputW && GridUtils::CanMoveInDirection(registry, player->currentNodeX,
				player->currentNodeY, Direction::Up, player->lastDirection)) {
				nextDir = Direction::Up;
			} else if (inputS && GridUtils::CanMoveInDirection(registry, player->currentNodeX,
				player->currentNodeY, Direction::Down, player->lastDirection)) {
				nextDir = Direction::Down;
			} else if (inputA && GridUtils::CanMoveInDirection(registry, player->currentNodeX,
				player->currentNodeY, Direction::Left, player->lastDirection)) {
				nextDir = Direction::Left;
			}
		} else if (player->currentDirection == Direction::Up) {
			// 上進行中 → 左右優先
			if (inputA && GridUtils::CanMoveInDirection(registry, player->currentNodeX,
				player->currentNodeY, Direction::Left, player->lastDirection)) {
				nextDir = Direction::Left;
			} else if (inputD && GridUtils::CanMoveInDirection(registry, player->currentNodeX,
				player->currentNodeY, Direction::Right, player->lastDirection)) {
				nextDir = Direction::Right;
			} else if (inputW && GridUtils::CanMoveInDirection(registry, player->currentNodeX,
				player->currentNodeY, Direction::Up, player->lastDirection)) {
				nextDir = Direction::Up;
			}
		} else if (player->currentDirection == Direction::Down) {
			// 下進行中 → 左右優先
			if (inputA && GridUtils::CanMoveInDirection(registry, player->currentNodeX,
				player->currentNodeY, Direction::Left, player->lastDirection)) {
				nextDir = Direction::Left;
			} else if (inputD && GridUtils::CanMoveInDirection(registry, player->currentNodeX,
				player->currentNodeY, Direction::Right, player->lastDirection)) {
				nextDir = Direction::Right;
			} else if (inputS && GridUtils::CanMoveInDirection(registry, player->currentNodeX,
				player->currentNodeY, Direction::Down, player->lastDirection)) {
				nextDir = Direction::Down;
			}
		}
	}

	// ========== 入力履歴をクリア ==========
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

	// 初回移動時のみ lastDirection を設定
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
	// エッジ途中で停止（progressOnEdge は保持）
	player->currentDirection = Direction::None;
	player->state = PlayerState::StoppedOnEdge;
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
		// ノード上
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

	// 接続数1 かつ 来た方向のみ = 行き止まり
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
	ImGui::Text("isMoving: %s", player->isMoving ? "true" : "false");
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