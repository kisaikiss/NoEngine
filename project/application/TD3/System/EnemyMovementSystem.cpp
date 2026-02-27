#include "EnemyMovementSystem.h"
#include "../GameTag.h"
#include "../Utility/GridUtils.h"
#include "../System/GameTimer.h"
#include <cmath>
#include <queue>
#include <set>
#include <utility>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

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

	// ========== ゲームタイマーから経過時間を取得 ==========
	float gameDeltaTime = 0.0f;
	if (gameTimer_) {
		gameDeltaTime = gameTimer_->GetGameDeltaTime();
	}

	// デバッグUI
#ifdef USE_IMGUI
	DebugUI(registry);
#endif

	auto enemyView = registry.View<EnemyComponent, EnemyTag, No::TransformComponent, DeathFlag>();

	// ========== ゲーム時間が進んでいない場合 ==========
	if (gameDeltaTime <= 0.0f) {
		for (auto entity : enemyView) {
			auto* enemy = registry.GetComponent<EnemyComponent>(entity);
			auto* transform = registry.GetComponent<No::TransformComponent>(entity);
			auto* deathFlag = registry.GetComponent<DeathFlag>(entity);
			if (deathFlag->isDead) continue;

			// reverseTimer の減算はリアルタイムで行う（ゲーム時間とは独立）
			if (enemy->reverseTimer > 0.0f) {
				enemy->reverseTimer -= deltaTime;
				if (enemy->reverseTimer < 0.0f) enemy->reverseTimer = 0.0f;
			}

			UpdateTransform(enemy, transform);
		}
		return;
	}

	int playerX = player->currentNodeX;
	int playerY = player->currentNodeY;

	for (auto entity : enemyView) {
		auto* enemy = registry.GetComponent<EnemyComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* deathFlag = registry.GetComponent<DeathFlag>(entity);

		// 死亡予定の敵は処理しない
		if (deathFlag->isDead) continue;

		// ---- reverseTimer 減算（リアルタイム） ----
		if (enemy->reverseTimer > 0.0f) {
			enemy->reverseTimer -= deltaTime;
			if (enemy->reverseTimer < 0.0f) enemy->reverseTimer = 0.0f;
		}

		switch (enemy->state) {
		case PlayerState::OnNode:
			HandleOnNode(enemy, playerX, playerY, registry);
			break;

		case PlayerState::MovingOnEdge:
			HandleOnEdge(enemy, gameDeltaTime, playerX, playerY, registry);
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
		// 余剰分を先に保持
		float overflow = enemy->progressOnEdge - 1.0f;
		
		// progressOnEdge を1.0に設定してからノード到達処理
		enemy->progressOnEdge = 1.0f;
		OnReachNode(enemy, playerX, playerY, registry);

		// 停止（方向が決まらなかった）場合はループ脱出
		if (enemy->state != PlayerState::MovingOnEdge) {
			break;
		}
		
		// 次の移動が開始された場合、余剰分を引き継ぐ
		// StartMovement内でprogressOnEdgeが0.0にリセットされているため、
		// ここで余剰分を加算する
		enemy->progressOnEdge += overflow;
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
//  ChooseDirection（BFS 最短経路探索）
//
//  敵の currentNode からプレイヤーの currentNode への最短経路を BFS で探索し、
//  「最初の1歩の方向」だけを返す。
//  ノード到達時に毎回呼ばれるため、1ステップ先だけ決めれば十分。
// ============================================================

Direction EnemyMovementSystem::ChooseDirection(
	EnemyComponent* enemy,
	int playerX, int playerY,
	No::Registry& registry
) {
	// ========== プレイヤーと完全に同じノードで、プレイヤーも停止中の場合のみ停止 ==========
	// プレイヤーが移動中の場合は targetNode を追いかける必要があるため、
	// ここでは停止しない
	if (enemy->currentNodeX == playerX && enemy->currentNodeY == playerY) {
		// プレイヤーの状態を確認
		auto playerView = registry.View<PlayerComponent, PlayerTag>();
		if (!playerView.Empty()) {
			auto it = playerView.begin();
			auto* player = registry.GetComponent<PlayerComponent>(*it);
			
			// プレイヤーが移動中の場合は targetNode を目標にする
			if (player && player->state == PlayerState::MovingOnEdge) {
				playerX = player->targetNodeX;
				playerY = player->targetNodeY;
			} else {
				// プレイヤーが停止中で同じノードにいる場合のみ停止
				return Direction::None;
			}
		}
	}

	// ========== BFS の準備 ==========

	using Coord = std::pair<int, int>;

	// BFS キューの要素
	// x, y : 現在チェックしているノードの座標
	// firstDir : スタートからの「最初の1歩の方向」（経路逆算の代わりに伝播させる）
	struct BFSNode {
		int x;
		int y;
		Direction firstDir;
	};

	std::queue<BFSNode> que;
	std::set<Coord>     visited;

	// スタートノードを訪問済みにする
	visited.insert({ enemy->currentNodeX, enemy->currentNodeY });

	// ========== 最初の1歩：後退禁止制約あり ==========
	// GridUtils::CanMoveInDirection に lastDirection を渡して
	// 後退禁止 + 行き止まり例外（A案）を適用する。
	const Direction allDirs[4] = {
		Direction::Up, Direction::Right, Direction::Down, Direction::Left
	};

	for (Direction dir : allDirs) {
		if (!GridUtils::CanMoveInDirection(
			registry,
			enemy->currentNodeX, enemy->currentNodeY,
			dir, enemy->lastDirection)) {
			continue;
		}

		int nx, ny;
		GridUtils::GetNextNodeCoords(enemy->currentNodeX, enemy->currentNodeY, dir, nx, ny);
		Coord nc = { nx, ny };

		if (visited.find(nc) == visited.end()) {
			visited.insert(nc);
			que.push({ nx, ny, dir });
		}
	}

	// ========== BFS 本体 ==========
	while (!que.empty()) {
		BFSNode current = que.front();
		que.pop();

		// ゴール到達 → 最初の1歩の方向を返す
		if (current.x == playerX && current.y == playerY) {
			return current.firstDir;
		}

		// ========== 2歩目以降：後退禁止制約なし ==========
		// Direction::None を lastDir に渡すことで CanMoveInDirection 内の
		// 後退禁止チェックをスキップし、接続確認のみを行う。
		for (Direction dir : allDirs) {
			if (!GridUtils::CanMoveInDirection(
				registry,
				current.x, current.y,
				dir, Direction::None)) {
				continue;
			}

			int nx, ny;
			GridUtils::GetNextNodeCoords(current.x, current.y, dir, nx, ny);
			Coord nc = { nx, ny };

			if (visited.find(nc) == visited.end()) {
				visited.insert(nc);
				// firstDir はスタートからの方向を引き継ぐ
				que.push({ nx, ny, current.firstDir });
			}
		}
	}

	// ========== 到達不能（孤立マップなど） ==========
	// 敵は停止する
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

	// ゲームタイマー情報の表示
	if (gameTimer_) {
		ImGui::Text("ゲームタイマー(プレイヤーが動いた時しか時間取らない)");
		ImGui::Text("Game Delta: %.4f sec", gameTimer_->GetGameDeltaTime());
		ImGui::Text("Time Scale: %.2fx", gameTimer_->GetTimeScale());
	} else {
		ImGui::Text("ゲームタイマーねぇよ");
	}
	ImGui::Separator();

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
		ImGui::Text("  ReverseTimer: %.2f", enemy->reverseTimer);
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