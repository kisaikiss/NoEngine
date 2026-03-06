#include "EnemyToEnemyCollisionSystem.h"
#include "../GameTag.h"
#include "../Component/ColliderComponent.h"
#include "../Utility/GridUtils.h"

// ============================================================
//  Update
// ============================================================

void EnemyToEnemyCollisionSystem::Update(No::Registry& registry, float deltaTime) {
	(void)deltaTime;

	auto view = registry.View<EnemyComponent, EnemyTag, SphereColliderComponent, DeathFlag>();


	for (auto entity : view) {
		auto* enemy = registry.GetComponent<EnemyComponent>(entity);
		auto* collider = registry.GetComponent<SphereColliderComponent>(entity);
		auto* death = registry.GetComponent<DeathFlag>(entity);

		// 死亡済みはスキップ
		if (death->isDead) continue;

		// スポーニング状態はスキップ（敵同士の衝突判定を行わない）
		if (enemy->isSpawning) continue;

		// クールダウン中はスキップ（振動防止）
		if (enemy->reverseTimer > 0.0f) continue;

		// CollisionSystem が検出した衝突相手が kEnemy かチェック
		if (!collider->isCollied) continue;
		if (collider->colliedWith != kEnemy) continue;

		// 衝突相手のエンティティが有効か確認
		NoEngine::ECS::Entity otherEntity = collider->colliedEntity;
		if (!registry.Has<EnemyComponent>(otherEntity)) continue;
		if (!registry.Has<DeathFlag>(otherEntity)) continue;

		auto* otherEnemy = registry.GetComponent<EnemyComponent>(otherEntity);
		auto* otherDeath = registry.GetComponent<DeathFlag>(otherEntity);

		if (otherDeath->isDead) continue;

		// 相手もスポーニング状態ならスキップ
		if (otherEnemy->isSpawning) continue;

		// 相手もクールダウン中なら両者ともスキップ（二重処理防止）
		if (otherEnemy->reverseTimer > 0.0f) continue;

		// ---- 両者を反転 ----
		ReverseEnemy(enemy);
		ReverseEnemy(otherEnemy);

		// クールダウンをセット（振動防止）
		enemy->reverseTimer = EnemyComponent::REVERSE_COOLDOWN;
		otherEnemy->reverseTimer = EnemyComponent::REVERSE_COOLDOWN;
	}
}

// ============================================================
//  ReverseEnemy
// ============================================================

void EnemyToEnemyCollisionSystem::ReverseEnemy(EnemyComponent* enemy) {

	// OnNode 上で衝突した場合はそのまま（次のChooseDirectionに任せる）
	if (enemy->state == PlayerState::OnNode) return;

	// currentNode ⇔ targetNode を入れ替える
	std::swap(enemy->currentNodeX, enemy->targetNodeX);
	std::swap(enemy->currentNodeY, enemy->targetNodeY);

	// progress を鏡像にする
	// → ワールド座標は変わらず、向きだけが逆になる
	enemy->progressOnEdge = 1.0f - enemy->progressOnEdge;

	// 移動方向を反転
	Direction reversed = GridUtils::GetOppositeDirection(enemy->currentDirection);
	enemy->currentDirection = reversed;
	enemy->actualMovingDirection = reversed;

	// lastDirection も更新しておかないと BFS の後退禁止判定がおかしくなる
	enemy->lastDirection = GridUtils::GetOppositeDirection(reversed);

	// 状態は MovingOnEdge のまま継続（止めない）
}