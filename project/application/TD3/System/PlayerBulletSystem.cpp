#include "PlayerBulletSystem.h"
#include <cmath>
#include "../GameTag.h"
#include "../Utility/GridUtils.h"
#include "../Utility/CameraBounds.h"
#include "../Component/EnemyComponent.h"
#include "../Component/HealthComponent.h"
#include "../Component/ColliderComponent.h"
#include "../Component/ShockwaveComponent.h"
#include "../Component/PlayerComponent.h"
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

					// ---- 敵貫通フラグのチェック ----
					if (!bullet->penetrateEnemies) {
						// 貫通しない場合は弾を消滅
						deathFlag->isDead = true;
						continue;
					} else {
						// 貫通する場合、敵ヒット時にループ無効化フラグをチェック
						if (bullet->disableLoopOnHit) {
							bullet->loopDisabled = true;
						}
					}
				}
			}
		}

		// ---- プレイヤーとの衝突判定（ループ後の弾のみ） ----
		if (bullet->loopedOnce) {
			HandlePlayerCollision(registry, entity, bullet);
			if (deathFlag->isDead) continue;
		}

		// ---- 画面外ループ処理 ----
		if (bullet->enableLooping && !bullet->loopDisabled) {
			HandleScreenLooping(registry, entity, bullet, transform);
			if (bullet->loopedOnce) {
				// ループ後はプレイヤーにも当たるようにマスクを変更
				collider->collideMask |= kPlayer;
			}
		}

		// ---- 交差点判定（壁貫通に関わらず実行） ----
		{
			// ワールド座標 → グリッド座標に変換する
			int nearestGridX, nearestGridY;
			GridUtils::WorldToGrid(transform->translate, nearestGridX, nearestGridY);

			// 最近接ノードのワールド座標を求め、そこからの距離で「ノード付近か」を判定する
			No::Vector3 nodeWorldPos = GridUtils::GridToWorld(nearestGridX, nearestGridY);
			float dx = transform->translate.x - nodeWorldPos.x;
			float dy = transform->translate.y - nodeWorldPos.y;
			float distFromNode = std::sqrt(dx * dx + dy * dy);

			// ノード付近の場合のみ処理
			if (distFromNode < NODE_DETECT_THRESHOLD) {
				// 発射元ノードのスキップ（発射直後のみ）
				bool isStartNode = (nearestGridX == bullet->startNodeX &&
					nearestGridY == bullet->startNodeY &&
					bullet->travelDistance < NODE_DETECT_THRESHOLD * 2.0f);

				if (!isStartNode) {
					// ---- 交差点判定：衝撃波を生成 ----
					int nodeHash = HashNodeCoords(nearestGridX, nearestGridY);
					if (bullet->visitedIntersections.count(nodeHash) == 0) {
						if (IsIntersectionNode(registry, nearestGridX, nearestGridY, bullet->direction)) {
							CreateShockwave(registry, nodeWorldPos);
							bullet->visitedIntersections.insert(nodeHash);
						}
					}
				}
			}
		}

		// ---- グリッドベース壁判定（penetrateWalls で制御） ----
		if (!bullet->penetrateWalls) {
			// ワールド座標 → グリッド座標に変換する
			int nearestGridX, nearestGridY;
			GridUtils::WorldToGrid(transform->translate, nearestGridX, nearestGridY);

			// 最近接ノードのワールド座標を求め、そこからの距離で「ノード付近か」を判定する
			No::Vector3 nodeWorldPos = GridUtils::GridToWorld(nearestGridX, nearestGridY);
			float dx = transform->translate.x - nodeWorldPos.x;
			float dy = transform->translate.y - nodeWorldPos.y;
			float distFromNode = std::sqrt(dx * dx + dy * dy);

			// ノード付近以外はスキップ
			if (distFromNode < NODE_DETECT_THRESHOLD) {
				// 発射元ノードのスキップ（発射直後のみ）
				bool isStartNode = (nearestGridX == bullet->startNodeX &&
					nearestGridY == bullet->startNodeY &&
					bullet->travelDistance < NODE_DETECT_THRESHOLD * 2.0f);

				if (!isStartNode) {
					// 前方接続チェック：接続がない or マップ外なら消滅
					if (ShouldDestroyAtNode(registry, nearestGridX, nearestGridY, bullet->direction)) {
						deathFlag->isDead = true;
					}
				}
			}
		}
	}
}

// ============================================================
//  HandleScreenLooping
// ============================================================

void PlayerBulletSystem::HandleScreenLooping(
	No::Registry& registry,
	No::Entity entity,
	PlayerBulletComponent* bullet,
	No::TransformComponent* transform
) {
	(void)registry;

	if (!camera_) return;

	// 既に1回ループしている場合
	if (bullet->loopedOnce) {
		// 2回目の画面外で消滅
		// ループ直後の境界判定を避けるため、完全に範囲外に出たことを確認
		float left, right, bottom, top;
		CameraBounds::GetVisibleBounds(camera_, transform->translate.z, left, right, bottom, top);
		
		// オフセット分範囲を広げる
		left -= bullet->screenBoundsOffset;
		right += bullet->screenBoundsOffset;
		bottom -= bullet->screenBoundsOffset;
		top += bullet->screenBoundsOffset;
		
		// 完全に範囲外に出た場合のみ消滅（境界上は許容）
		bool completelyOutside = (transform->translate.x < left - 1.0f || 
		                          transform->translate.x > right + 1.0f ||
		                          transform->translate.y < bottom - 1.0f || 
		                          transform->translate.y > top + 1.0f);
		
		if (completelyOutside) {
			auto* deathFlag = registry.GetComponent<DeathFlag>(entity);
			if (deathFlag) {
				deathFlag->isDead = true;
			}
		}
		return;
	}

	// まだループしていない場合、ループ処理を実行
	bool looped = false;
	No::Vector3 newPosition = CameraBounds::LoopPosition(
		camera_,
		transform->translate,
		bullet->screenBoundsOffset,
		looped
	);

	if (looped) {
		transform->translate = newPosition;
		bullet->loopedOnce = true;
		// ループ時に訪問済み交差点をクリア（ループ後も衝撃波を発生させるため）
		bullet->visitedIntersections.clear();
	}
}

// ============================================================
//  HandlePlayerCollision
// ============================================================

void PlayerBulletSystem::HandlePlayerCollision(
	No::Registry& registry,
	No::Entity bulletEntity,
	PlayerBulletComponent* bullet
) {
	(void)bullet;

	auto* bulletCollider = registry.GetComponent<SphereColliderComponent>(bulletEntity);
	if (!bulletCollider) return;

	// プレイヤーとの衝突判定
	if (bulletCollider->isCollied && bulletCollider->colliedWith == kPlayer) {
		auto playerEntity = bulletCollider->colliedEntity;

		// プレイヤーが有効か確認
		if (registry.Has<HealthComponent>(playerEntity) && registry.Has<DeathFlag>(playerEntity)) {
			auto* playerHealth = registry.GetComponent<HealthComponent>(playerEntity);
			auto* playerDeath = registry.GetComponent<DeathFlag>(playerEntity);

			if (!playerDeath->isDead) {
				bool died = playerHealth->TakeDamage(1);
				if (died) {
					playerDeath->isDead = true;
				}

				// 弾を消滅
				auto* bulletDeath = registry.GetComponent<DeathFlag>(bulletEntity);
				if (bulletDeath) {
					bulletDeath->isDead = true;
				}
			}
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
	(void)direction;  // 方向は使用しない
	
	auto* cell = GridUtils::GetGridCell(registry, nodeX, nodeY);
	if (!cell) return false;

	// 全方向の接続数をカウント
	int totalConnections = 0;
	if (cell->hasConnectionUp) totalConnections++;
	if (cell->hasConnectionDown) totalConnections++;
	if (cell->hasConnectionLeft) totalConnections++;
	if (cell->hasConnectionRight) totalConnections++;

	// 接続が3以上 = 交差点（T字、十字路、それ以上）
	if (totalConnections >= 3) {
		return true;
	}

	// 接続が2の場合 = 直線道 または L字
	if (totalConnections == 2) {
		// 縦方向と横方向の両方に接続がある = L字（曲がり角）
		bool hasVertical = (cell->hasConnectionUp || cell->hasConnectionDown);
		bool hasHorizontal = (cell->hasConnectionLeft || cell->hasConnectionRight);
		
		// 縦横両方に接続があればL字（曲がり角）
		if (hasVertical && hasHorizontal) {
			return true;
		}
		
		// 同じ軸（縦のみ or 横のみ）= 直線道
		return false;
	}

	// 接続が1以下 = 行き止まり or 孤立ノード
	return false;
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