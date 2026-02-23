#include "EnemyMovementSystem.h"
#include "../GameTag.h"
#include "../Utility/GridUtils.h"
#include <cmath>
#include <cstdlib>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

// ============================================================
//  Update
// ============================================================

void EnemyMovementSystem::Update(No::Registry& registry, float deltaTime) {

	auto playerView = registry.View<PlayerComponent, PlayerTag>();
	if (playerView.Empty()) return;

	PlayerComponent* player = nullptr;
	{
		// Empty() チェック済みのため begin() は必ず有効
		auto it = playerView.begin();
		player = registry.GetComponent<PlayerComponent>(*it);
	}
	if (!player) return;

	// デバッグUI
#ifdef USE_IMGUI
	DebugUI(registry);
#endif

	/// プレイヤーが動いていないときは敵も動かさない
	if (!player->isMoving) return;

	int playerX = player->currentNodeX;
	int playerY = player->currentNodeY;

	auto enemyView = registry.View<EnemyComponent, EnemyTag, No::TransformComponent, DeathFlag>();
	for (auto entity : enemyView) {
		auto* enemy = registry.GetComponent<EnemyComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* deathFlag = registry.GetComponent<DeathFlag>(entity);

		// 死亡予定の敵は処理しない
		if (deathFlag->isDead) continue;

		switch (enemy->state) {
		case PlayerState::OnNode:
			HandleOnNode(enemy, playerX, playerY, registry);
			break;

		case PlayerState::MovingOnEdge:
			HandleOnEdge(enemy, deltaTime, playerX, playerY, registry);
			break;

		case PlayerState::StoppedOnEdge:
			// 敵はエッジ上で止まらない設計だが、念のため OnNode に戻して再開する
			enemy->state = PlayerState::OnNode;
			HandleOnNode(enemy, playerX, playerY, registry);
			break;
		}

		// Transform 更新
		UpdateTransform(enemy, transform);
	}
}

// ============================================================
//  HandleOnNode
// ============================================================

void EnemyMovementSystem::HandleOnNode(
	EnemyComponent* enemy,
	int playerX, int playerY,
	No::Registry& registry
) {
	Direction nextDir = ChooseDirection(enemy, playerX, playerY, registry);
	if (nextDir != Direction::None) {
		StartMovement(enemy, nextDir);
	}
}

// ============================================================
//  HandleOnEdge
// ============================================================

void EnemyMovementSystem::HandleOnEdge(
	EnemyComponent* enemy,
	float deltaTime,
	int playerX, int playerY,
	No::Registry& registry
) {
	enemy->progressOnEdge += enemy->moveSpeed * deltaTime;

	// ノード到達チェック（高速移動対応）
	while (enemy->progressOnEdge >= 1.0f) {
		enemy->progressOnEdge -= 1.0f;
		OnReachNode(enemy, playerX, playerY, registry);

		// 停止（方向が決まらなかった）場合はループ脱出
		if (enemy->state != PlayerState::MovingOnEdge) {
			break;
		}
	}
}

// ============================================================
//  OnReachNode
// ============================================================

void EnemyMovementSystem::OnReachNode(
	EnemyComponent* enemy,
	int playerX, int playerY,
	No::Registry& registry
) {
	// 現在座標をターゲットノードに更新
	enemy->currentNodeX = enemy->targetNodeX;
	enemy->currentNodeY = enemy->targetNodeY;

	// 来た方向を記録（後退禁止判定に使用）
	if (enemy->currentDirection != Direction::None) {
		enemy->lastDirection = enemy->currentDirection;
	}

	enemy->state = PlayerState::OnNode;

	// 次の移動方向を決定
	Direction nextDir = ChooseDirection(enemy, playerX, playerY, registry);
	if (nextDir != Direction::None) {
		StartMovement(enemy, nextDir);
	}
}

// ============================================================
//  StartMovement
// ============================================================

void EnemyMovementSystem::StartMovement(
	EnemyComponent* enemy,
	Direction dir
) {
	// 初回移動時のみ lastDirection を設定
	if (enemy->lastDirection == Direction::None) {
		enemy->lastDirection = dir;
	}

	enemy->actualMovingDirection = dir;
	enemy->currentDirection = dir;
	enemy->state = PlayerState::MovingOnEdge;
	enemy->progressOnEdge = 0.0f;

	// ターゲットノード座標を計算
	GridUtils::GetNextNodeCoords(
		enemy->currentNodeX, enemy->currentNodeY,
		dir,
		enemy->targetNodeX, enemy->targetNodeY
	);
}

// ============================================================
//  ChooseDirection（Stage3: グリーディーヒューリスティック）
//
//  Stage4 でこの関数を BFS 最短経路探索に差し替える。
//  BFS に切り替えても呼び出し元（OnReachNode / HandleOnNode）は変更不要。
// ============================================================

Direction EnemyMovementSystem::ChooseDirection(
	EnemyComponent* enemy,
	int playerX, int playerY,
	No::Registry& registry
) {
	int dx = playerX - enemy->currentNodeX;
	int dy = playerY - enemy->currentNodeY;

	// プレイヤー方向への優先リストを構築
	// マンハッタン距離が大きい軸を先に試みる
	Direction pref[4];
	int count = 0;

	if (std::abs(dx) >= std::abs(dy)) {
		// X 軸距離が大きい（or 等しい）場合、X 方向を先に試す
		if (dx > 0 && count < 4) pref[count++] = Direction::Right;
		else if (dx < 0 && count < 4) pref[count++] = Direction::Left;
		if (dy > 0 && count < 4) pref[count++] = Direction::Up;
		else if (dy < 0 && count < 4) pref[count++] = Direction::Down;
	} else {
		// Y 軸距離が大きい場合、Y 方向を先に試す
		if (dy > 0 && count < 4) pref[count++] = Direction::Up;
		else if (dy < 0 && count < 4) pref[count++] = Direction::Down;
		if (dx > 0 && count < 4) pref[count++] = Direction::Right;
		else if (dx < 0 && count < 4) pref[count++] = Direction::Left;
	}

	// プレイヤーと同一ノードなど残り方向を補完する
	Direction all[4] = { Direction::Up, Direction::Right, Direction::Down, Direction::Left };
	for (int i = 0; i < 4; i++) {
		bool found = false;
		for (int j = 0; j < count; j++) {
			if (pref[j] == all[i]) { found = true; break; }
		}
		if (!found && count < 4) pref[count++] = all[i];
	}

	// 優先順に GridUtils::CanMoveInDirection で移動可否を確認
	// 後退禁止 + 行き止まり例外（A案）が適用される
	for (int i = 0; i < count; i++) {
		if (GridUtils::CanMoveInDirection(
			registry,
			enemy->currentNodeX, enemy->currentNodeY,
			pref[i], enemy->lastDirection)) {
			return pref[i];
		}
	}

	return Direction::None;
}

// ============================================================
//  CalculateWorldPosition
// ============================================================

No::Vector3 EnemyMovementSystem::CalculateWorldPosition(
	const EnemyComponent* enemy
) {
	if (enemy->state == PlayerState::OnNode) {
		return GridUtils::GridToWorld(enemy->currentNodeX, enemy->currentNodeY);
	}

	// エッジ上を線形補間（MovingOnEdge）
	No::Vector3 from = GridUtils::GridToWorld(enemy->currentNodeX, enemy->currentNodeY);
	No::Vector3 to = GridUtils::GridToWorld(enemy->targetNodeX, enemy->targetNodeY);
	return from + (to - from) * enemy->progressOnEdge;
}

// ============================================================
//  UpdateTransform
// ============================================================

void EnemyMovementSystem::UpdateTransform(
	const EnemyComponent* enemy,
	No::TransformComponent* transform
) {
	// 位置のみ更新する（敵のモデル回転は未実装）
	transform->translate = CalculateWorldPosition(enemy);
}

// ============================================================
//  デバッグ UI
// ============================================================

#ifdef USE_IMGUI
void EnemyMovementSystem::DebugUI(No::Registry& registry) {
	ImGui::Begin("Enemy Movement");

	auto view = registry.View<EnemyComponent, EnemyTag>();
	int idx = 0;

	for (auto entity : view) {
		auto* enemy = registry.GetComponent<EnemyComponent>(entity);

		ImGui::Text("Enemy [%d]", idx++);
		ImGui::Text("  Current Node: (%d, %d)", enemy->currentNodeX, enemy->currentNodeY);

		if (enemy->state == PlayerState::MovingOnEdge) {
			ImGui::Text("  Target Node: (%d, %d)", enemy->targetNodeX, enemy->targetNodeY);
			ImGui::ProgressBar(enemy->progressOnEdge, ImVec2(-1, 0), "Progress");
		}

		ImGui::Text("  Direction: %s", DirectionToString(enemy->actualMovingDirection));
		ImGui::Text("  Last Direction: %s", DirectionToString(enemy->lastDirection));
		ImGui::DragFloat("Speed##enemy", &enemy->moveSpeed, 0.1f, 0.1f, 10.0f);
		ImGui::Separator();
	}

	if (idx == 0) {
		ImGui::Text("(敵が存在しません)");
	}

	ImGui::End();
}

const char* EnemyMovementSystem::DirectionToString(Direction dir) {
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