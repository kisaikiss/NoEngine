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
#include "../Component/DeathEffectComponent.h"
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
					// スポーニング状態の敵は弾ダメージを受けない（弾も消えない）
					bool enemyIsSpawning = false;
					if (registry.Has<EnemyComponent>(enemyEntity)) {
						enemyIsSpawning = registry.GetComponent<EnemyComponent>(enemyEntity)->isSpawning;
					}

					if (!enemyIsSpawning) {
						bool died = enemyHealth->TakeDamage(1);
						if (died) {
							// 敵の位置を取得して撃破演出を生成
							if (registry.Has<No::TransformComponent>(enemyEntity)) {
								auto* enemyTransform = registry.GetComponent<No::TransformComponent>(enemyEntity);

								// 敵撃破演出を生成
								DeathEffectConfig config;
								config.color = { 1.0f, 0.2f, 0.2f, 1.0f };  // 赤
								config.particleScale = 0.15f;
								config.particleCount = 8;
								config.duration = 0.6f;
								config.spreadDistance = 2.0f;

								DeathEffectHelper::SpawnDeathEffect(registry, enemyTransform->translate, config);
							}

							enemyDeath->isDead = true;
							// 敵撃破カウント増加
							if (onEnemyKilled_) {
								onEnemyKilled_();
							}
						}

						if (!bullet->penetrateEnemies) {
							deathFlag->isDead = true;
							continue;
						}
					}
				}
			}
		}

		// ---- プレイヤーとの衝突判定（ループ弾のみ） ----
		if (bullet->isLoopedBullet) {
			HandlePlayerCollision(registry, entity, bullet);
			if (deathFlag->isDead) continue;
		}

		// ---- 画面外ループ処理 ----
		if (bullet->enableLooping && camera_) {
			HandleScreenLooping(registry, entity, bullet, transform, deathFlag);
			if (deathFlag->isDead) continue;
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
	No::TransformComponent* transform,
	DeathFlag* deathFlag
) {
	(void)entity;

	bool isOutside = !CameraBounds::IsInBounds(camera_, transform->translate, bullet->screenBoundsOffset);

	if (isOutside) {
		if (!bullet->isLoopedBullet) {
			// 通常弾が画面外 → ループ弾を生成
			// AddComponent がストレージを再アロケートすると transform・bullet ポインタが無効になるため、
			// 事前に必要な値をコピーしてからループ弾を生成する（ダングリングポインタ防止）
			No::Vector3 currentPosition = transform->translate;
			PlayerBulletComponent bulletCopy = *bullet;
			No::Vector3 loopedPosition = CalculateLoopedPosition(currentPosition, bulletCopy.screenBoundsOffset);
			CreateLoopedBullet(registry, loopedPosition, bulletCopy);
		}
		// 元の弾を削除
		deathFlag->isDead = true;
	}
}

// ============================================================
//  CalculateLoopedPosition
// ============================================================

No::Vector3 PlayerBulletSystem::CalculateLoopedPosition(const No::Vector3& currentPosition, float offset) {
	if (!camera_) return currentPosition;

	float left, right, bottom, top;
	CameraBounds::GetVisibleBounds(camera_, currentPosition.z, left, right, bottom, top);

	left -= offset;
	right += offset;
	bottom -= offset;
	top += offset;

	No::Vector3 loopedPosition = currentPosition;

	if (currentPosition.x < left) {
		loopedPosition.x = right;
	} else if (currentPosition.x > right) {
		loopedPosition.x = left;
	}

	if (currentPosition.y < bottom) {
		loopedPosition.y = top;
	} else if (currentPosition.y > top) {
		loopedPosition.y = bottom;
	}

	return loopedPosition;
}

// ============================================================
//  CreateLoopedBullet
// ============================================================

void PlayerBulletSystem::CreateLoopedBullet(
	No::Registry& registry,
	const No::Vector3& loopedPosition,
	const PlayerBulletComponent originalBullet	// 値渡し：AddComponent による再アロケートでポインタが無効になるのを防ぐ
) {
	auto bulletEntity = registry.GenerateEntity();

	registry.AddComponent<PlayerBulletTag>(bulletEntity);
	registry.AddComponent<DeathFlag>(bulletEntity);

	auto* bullet = registry.AddComponent<PlayerBulletComponent>(bulletEntity);
	bullet->direction = originalBullet.direction;
	bullet->speed = originalBullet.speed;
	bullet->penetrateWalls = originalBullet.penetrateWalls;
	bullet->penetrateEnemies = originalBullet.penetrateEnemies;
	bullet->enableLooping = originalBullet.enableLooping;
	bullet->disableLoopOnHit = originalBullet.disableLoopOnHit;
	bullet->screenBoundsOffset = originalBullet.screenBoundsOffset;
	bullet->startNodeX = originalBullet.startNodeX;
	bullet->startNodeY = originalBullet.startNodeY;
	bullet->travelDistance = 0.0f;
	bullet->isLoopedBullet = true;

	auto* collider = registry.AddComponent<SphereColliderComponent>(bulletEntity);
	collider->radius = 0.5f;
	collider->colliderType = kPlayerBullet;
	collider->collideMask = kEnemy | kPlayer;

	auto* transform = registry.AddComponent<No::TransformComponent>(bulletEntity);
	transform->translate = loopedPosition;
	transform->scale = { 0.2f, 0.2f, 0.2f };

	auto* mesh = registry.AddComponent<No::MeshComponent>(bulletEntity);
	auto* material = registry.AddComponent<No::MaterialComponent>(bulletEntity);
	NoEngine::Asset::ModelLoader::LoadModel(
		"PlayerBullet",
		"resources/game/td_3105/Model/Shot/Shot.obj",
		mesh
	);
	material->materials = NoEngine::Asset::ModelLoader::GetMaterial("PlayerBullet");
	material->color = { 1.0f, 0.5f, 0.0f, 1.0f };
	material->psoName = L"Renderer : Default PSO";
	material->psoId = NoEngine::Render::GetPSOID(material->psoName);
	material->rootSigId = NoEngine::Render::GetRootSignatureID(material->psoName);
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

			// プレイヤーにダメージを与える
			bool died = playerHealth->TakeDamage(1);
			if (died && !playerHealth->isInvincible) {
				playerDeath->isDead = true;
			} else {
				// プレイヤーがダメージを受けた（死亡していない）場合、プレイヤーダメージ演出を生成
				if (registry.Has<No::TransformComponent>(playerEntity)) {
					auto* playerTransform = registry.GetComponent<No::TransformComponent>(playerEntity);

					// プレイヤーダメージ演出を生成（色違い）
					DeathEffectConfig playerDamageConfig;
					playerDamageConfig.color = { 0.3f, 0.5f, 1.0f, 1.0f };  // 青
					playerDamageConfig.particleScale = 0.3f;
					playerDamageConfig.particleCount = 6;
					playerDamageConfig.duration = 0.5f;
					playerDamageConfig.spreadDistance = 1.5f;

					DeathEffectHelper::SpawnDeathEffect(registry, playerTransform->translate, playerDamageConfig);

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