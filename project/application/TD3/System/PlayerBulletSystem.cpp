#include "PlayerBulletSystem.h"
#include <cmath>
#include "../GameTag.h"
#include "../Utility/GridUtils.h"
#include "../Component/EnemyComponent.h"
#include "../Component/HealthComponent.h"
#include "../Component/ColliderComponent.h"
#include "../Component/ShockwaveComponent.h"
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
	auto view = registry.View<PlayerBulletComponent, PlayerBulletTag, DeathFlag, No::TransformComponent, SphereColliderComponent>();
	if (view.Empty())return;

	for (auto entity : view) {
		auto* bullet = registry.GetComponent<PlayerBulletComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* deathFlag = registry.GetComponent<DeathFlag>(entity);
		auto* collider = registry.GetComponent<SphereColliderComponent>(entity);

		// ---- 移動 ----
		No::Vector3 movement = bullet->direction * bullet->speed * deltaTime;
		transform->translate = transform->translate + movement;
		bullet->travelDistance += bullet->speed * deltaTime;

		// ---- 安全網：最大距離で消滅 ----
		if (bullet->travelDistance >= bullet->maxDistance) {
			deathFlag->isDead = true;
			continue;
		}

		// ---- 敵との衝突判定（CollisionSystemの結果を参照） ----
		if (collider->isCollied && collider->colliedWith == kEnemy) {
			auto enemyEntity = collider->colliedEntity;

			// 敵が有効か確認
			if (registry.Has<HealthComponent>(enemyEntity) && registry.Has<DeathFlag>(enemyEntity)) {
				auto* enemyHealth = registry.GetComponent<HealthComponent>(enemyEntity);
				auto* enemyDeath = registry.GetComponent<DeathFlag>(enemyEntity);

				if (!enemyDeath->isDead) {
					bool died = enemyHealth->TakeDamage(1);
					if (died) {
						enemyDeath->isDead = true;
					}
					// 弾を消滅
					deathFlag->isDead = true;
					continue;
				}
			}
		}

		// ---- グリッドベース壁判定 ----
		// 始点スキップは座標ベースで行う（距離ベースにするとエッジ途中発射時にバグが起きる）

		// ワールド座標 → グリッド座標に変換する
		// grid_scale=1.0 以外でも正しく動作させるために GridUtils::WorldToGrid を使う
		// （以前は std::round(translate.x) をそのままグリッド座標として使っており、scale=2.0 のステージでノードが見つからず即消滅するバグがあった）
		int nearestGridX, nearestGridY;
		GridUtils::WorldToGrid(transform->translate, nearestGridX, nearestGridY);

		// 最近接ノードのワールド座標を求め、そこからの距離で「ノード付近か」を判定する
		No::Vector3 nodeWorldPos = GridUtils::GridToWorld(nearestGridX, nearestGridY);
		float dx = transform->translate.x - nodeWorldPos.x;
		float dy = transform->translate.y - nodeWorldPos.y;
		float distFromNode = std::sqrt(dx * dx + dy * dy);

		// ノード付近以外はスキップ
		if (distFromNode >= NODE_DETECT_THRESHOLD) {
			continue;
		}

		// 発射元ノードのスキップ（発射直後のみ）
		// （行き止まりノードで発射した場合に素通りしてしまうバグへの対処）
		if (nearestGridX == bullet->startNodeX &&
			nearestGridY == bullet->startNodeY &&
			bullet->travelDistance < NODE_DETECT_THRESHOLD * 2.0f) {
			continue;
		}

		// ---- 交差点判定：衝撃波を生成 ----
		int nodeHash = HashNodeCoords(nearestGridX, nearestGridY);
		if (bullet->visitedIntersections.count(nodeHash) == 0) {
			if (IsIntersectionNode(registry, nearestGridX, nearestGridY, bullet->direction)) {
				// 衝撃波を生成
				CreateShockwave(registry, nodeWorldPos);
				// 訪問済みとしてマーク
				bullet->visitedIntersections.insert(nodeHash);
			}
		}

		// 前方接続チェック：接続がない or マップ外なら消滅
		if (ShouldDestroyAtNode(registry, nearestGridX, nearestGridY, bullet->direction)) {
			deathFlag->isDead = true;
		}
	}
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

// ============================================================
//  IsIntersectionNode
// ============================================================

bool PlayerBulletSystem::IsIntersectionNode(
	No::Registry& registry,
	int nodeX, int nodeY,
	const No::Vector3& direction
) {
	auto* cell = GridUtils::GetGridCell(registry, nodeX, nodeY);
	if (!cell) return false;

	// 進行方向を判定
	bool goingUp = (direction.y > 0.5f);
	bool goingDown = (direction.y < -0.5f);
	bool goingRight = (direction.x > 0.5f);
	bool goingLeft = (direction.x < -0.5f);

	// 進行方向と来た方向（逆方向）以外の接続をカウント
	int sideConnections = 0;

	if (goingUp || goingDown) {
		// 縦方向に進んでいる場合、横方向の接続をチェック
		if (cell->hasConnectionLeft) sideConnections++;
		if (cell->hasConnectionRight) sideConnections++;
	} else if (goingRight || goingLeft) {
		// 横方向に進んでいる場合、縦方向の接続をチェック
		if (cell->hasConnectionUp) sideConnections++;
		if (cell->hasConnectionDown) sideConnections++;
	}

	// 横道が1つ以上あれば交差点（曲がり角）とみなす
	return sideConnections > 0;
}

// ============================================================
//  CreateShockwave
// ============================================================

void PlayerBulletSystem::CreateShockwave(No::Registry& registry, const No::Vector3& position) {
	auto shockwaveEntity = registry.GenerateEntity();

	// Transform
	auto* transform = registry.AddComponent<No::TransformComponent>(shockwaveEntity);
	transform->translate = position;
	transform->scale = No::Vector3{ 1.0f, 1.0f, 1.0f };

	// Shockwave Component
	auto* shockwave = registry.AddComponent<ShockwaveComponent>(shockwaveEntity);
	// デフォルト値を使用（パラメータは ShockwaveComponent のコンストラクタで設定）

	// Collider（初期サイズは minRadius）
	auto* collider = registry.AddComponent<SphereColliderComponent>(shockwaveEntity);
	collider->radius = shockwave->minRadius;
	collider->worldRadius = shockwave->minRadius;
	collider->colliderType = kShockwave;
	collider->collideMask = kEnemy;  // 敵とのみ衝突

	// Tag
	registry.AddComponent<ShockwaveTag>(shockwaveEntity);

	// DeathFlag
	registry.AddComponent<DeathFlag>(shockwaveEntity);
}

// ============================================================
//  HashNodeCoords
// ============================================================

int PlayerBulletSystem::HashNodeCoords(int x, int y) const {
	// 簡易的なハッシュ関数（Cantor pairing function の変形）
	return (x + y) * (x + y + 1) / 2 + y;
}