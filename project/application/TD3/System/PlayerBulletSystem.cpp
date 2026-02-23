#include "PlayerBulletSystem.h"
#include <cmath>
#include "../GameTag.h"
#include "../Utility/GridUtils.h"
#include "../Component/EnemyComponent.h"
#include "../Component/HealthComponent.h"
#include "engine/Functions/Renderer/Primitive.h"


// ノード付近と判定する距離の閾値
// グリッド間隔が 1.0 のため 0.15f で十分に検出できる。
// 弾速 5.0f・60fps 時の 1 フレームの移動量は約 0.083f なので
// ノードを通過しても必ず 1〜2 フレーム以内に検出される。
static constexpr float NODE_DETECT_THRESHOLD = 0.15f;

// ============================================================
//  Update
// ============================================================

void PlayerBulletSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<PlayerBulletComponent, PlayerBulletTag, DeathFlag, No::TransformComponent>();

	for (auto entity : view) {
		auto* bullet = registry.GetComponent<PlayerBulletComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* deathFlag = registry.GetComponent<DeathFlag>(entity);

		// ---- 移動 ----
		No::Vector3 movement = bullet->direction * bullet->speed * deltaTime;
		transform->translate = transform->translate + movement;
		bullet->travelDistance += bullet->speed * deltaTime;
		NoEngine::Primitive::DrawSphere(transform->translate, transform->scale.x, { 0.0f, 0.0f, 0.0f, 1.0f });

		// ---- 安全網：最大距離で消滅 ----
		if (bullet->travelDistance >= bullet->maxDistance) {
			deathFlag->isDead = true;
			continue;
		}

		// ---- グリッドベース消滅判定 ----
		// 始点スキップは座標ベースで行う（距離ベースにするとエッジ途中発射時にバグが起きる）

		int nearestX = static_cast<int>(std::round(transform->translate.x));
		int nearestY = static_cast<int>(std::round(transform->translate.y));

		float dx = transform->translate.x - static_cast<float>(nearestX);
		float dy = transform->translate.y - static_cast<float>(nearestY);
		float distFromNode = std::sqrt(dx * dx + dy * dy);

		// ノード付近以外はスキップ
		if (distFromNode >= NODE_DETECT_THRESHOLD) {
			continue;
		}

		// 発射元ノードのスキップ（発射直後のみ）
		// （行き止まりノードで発射した場合に素通りしてしまうバグへの対処）
		if (nearestX == bullet->startNodeX &&
			nearestY == bullet->startNodeY &&
			bullet->travelDistance < NODE_DETECT_THRESHOLD * 2.0f) {
			continue;
		}

		// 敵ヒット判定（壁チェックより先に実行）
		// 弾がノードに到達したとき、まず敵がいるかチェックする。
		// 敵にヒットした場合は壁判定をスキップして即座に弾を消滅させる。
		if (CheckEnemyHitAtNode(registry, nearestX, nearestY)) {
			deathFlag->isDead = true;
			continue;
		}

		// 前方接続チェック：接続がない or マップ外なら消滅
		if (ShouldDestroyAtNode(registry, nearestX, nearestY, bullet->direction)) {
			deathFlag->isDead = true;
		}
	}
}

// ============================================================
//  CheckEnemyHitAtNode
// ============================================================

bool PlayerBulletSystem::CheckEnemyHitAtNode(
	No::Registry& registry,
	int nodeX,
	int nodeY
) {
	auto view = registry.View<EnemyComponent, EnemyTag, DeathFlag, HealthComponent>();
	if (view.Empty()) return false;

	for (auto entity : view) {
		auto* enemy = registry.GetComponent<EnemyComponent>(entity);
		auto* enemyDeath = registry.GetComponent<DeathFlag>(entity);
		auto* health = registry.GetComponent<HealthComponent>(entity);

		// すでに死亡予定の敵はスキップ
		if (enemyDeath->isDead) continue;

		// 敵の currentNode が弾のノード座標と一致するかチェック
		if (enemy->currentNodeX == nodeX && enemy->currentNodeY == nodeY) {
			bool died = health->TakeDamage(1);
			if (died) {
				enemyDeath->isDead = true;
			}
			return true; // ヒットしたら弾を消滅させる
		}
	}
	return false;
}

// ============================================================
//  ShouldDestroyAtNode
// ============================================================

bool PlayerBulletSystem::ShouldDestroyAtNode(
	No::Registry& registry,
	int nodeX, int nodeY,
	const No::Vector3& direction
) {
	auto* cell = GridUtils::GetGridCell(registry, nodeX, nodeY);

	// マップ外なら消滅
	if (!cell) return true;

	// 進行方向ベクトルから方向を判定し、前方接続をチェック
	bool hasForwardConnection = false;

	if (direction.y > 0.5f) {
		hasForwardConnection = cell->hasConnectionUp;
	} else if (direction.y < -0.5f) {
		hasForwardConnection = cell->hasConnectionDown;
	} else if (direction.x > 0.5f) {
		hasForwardConnection = cell->hasConnectionRight;
	} else if (direction.x < -0.5f) {
		hasForwardConnection = cell->hasConnectionLeft;
	}

	// 前方への接続がない場合消滅
	return !hasForwardConnection;
}