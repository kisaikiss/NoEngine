#include "ShockwaveSystem.h"
#include "../Component/ShockwaveComponent.h"
#include "../Component/HealthComponent.h"
#include "../Component/ColliderComponent.h"
#include "../Component/EnemyComponent.h"
#include "../GameTag.h"
#include "engine/Math/Easing.h"
#include "engine/Functions/Renderer/Primitive.h"

using namespace NoEngine::Easing;

// ============================================================
//  Update
// ============================================================

void ShockwaveSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<ShockwaveComponent, ShockwaveTag, DeathFlag, No::TransformComponent, SphereColliderComponent>();
	if (view.Empty()) return;

	for (auto entity : view) {
		auto* shockwave = registry.GetComponent<ShockwaveComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* deathFlag = registry.GetComponent<DeathFlag>(entity);
		auto* collider = registry.GetComponent<SphereColliderComponent>(entity);

		// 経過時間を更新
		shockwave->elapsedTime += deltaTime;

		// ---- Phase 1: 拡大フェーズ ----
		if (shockwave->elapsedTime < shockwave->expandDuration) {
			float t = shockwave->elapsedTime / shockwave->expandDuration;
			shockwave->currentRadius = EaseOutCubic(shockwave->minRadius, shockwave->maxRadius, t);
			shockwave->currentAlpha = 1.0f;
		}
		// ---- Phase 2: フェードアウトフェーズ ----
		else if (shockwave->elapsedTime < shockwave->expandDuration + shockwave->fadeDuration) {
			float t = (shockwave->elapsedTime - shockwave->expandDuration) / shockwave->fadeDuration;
			shockwave->currentRadius = shockwave->maxRadius;
			shockwave->currentAlpha = Lerp(1.0f, 0.0f, t);
		}
		// ---- Phase 3: 消滅 ----
		else {
			deathFlag->isDead = true;
			continue;
		}

		// コライダーの半径を更新
		collider->radius = shockwave->currentRadius;
		collider->worldRadius = shockwave->currentRadius * transform->scale.x;

		// ---- 敵との衝突判定 ----
		if (collider->isCollied && collider->colliedWith == kEnemy) {
			auto enemyEntity = collider->colliedEntity;

			// 既にヒット済みならスキップ
			if (shockwave->hitEnemies.find(enemyEntity) != shockwave->hitEnemies.end()) {
				continue;
			}

			// 敵が有効か確認
			if (registry.Has<HealthComponent>(enemyEntity) && registry.Has<DeathFlag>(enemyEntity)) {
				auto* enemyHealth = registry.GetComponent<HealthComponent>(enemyEntity);
				auto* enemyDeath = registry.GetComponent<DeathFlag>(enemyEntity);

				if (!enemyDeath->isDead) {
					// スポーニング状態の敵は衝撃波を受けない
					if (registry.Has<EnemyComponent>(enemyEntity)) {
						auto* ec = registry.GetComponent<EnemyComponent>(enemyEntity);
						if (ec->isSpawning) continue;
					}

					// ダメージを与える
					bool died = enemyHealth->TakeDamage(1);
					if (died) {
						enemyDeath->isDead = true;
						// 敵撃破カウント増加
						if (onEnemyKilled_) {
							onEnemyKilled_();
						}
					}
					// ヒット済みとしてマーク
					shockwave->hitEnemies.insert(enemyEntity);

#ifdef USE_IMGUI
					NoEngine::Primitive::DrawSphere(transform->translate, shockwave->currentRadius * 1.2f,
						{ 1.0f, 0.0f, 0.0f, 0.8f });
#endif
				}
			}
		}

		// ---- デバッグ描画 ----
#ifdef USE_IMGUI
		// 衝撃波を青色の半透明円で描画
		No::Color color = collider->isCollied ?
			No::Color{ 1.0f, 1.0f, 0.0f, shockwave->currentAlpha * 0.7f } :  // 衝突中は黄色
			No::Color{ 0.0f, 0.0f, 0.0f, shockwave->currentAlpha * 0.5f };   // 通常は青
		NoEngine::Primitive::DrawSphere(transform->translate, shockwave->currentRadius, color);
#endif
	}
}