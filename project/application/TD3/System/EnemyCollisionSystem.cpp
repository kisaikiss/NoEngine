#include "EnemyCollisionSystem.h"
#include "../GameTag.h"
#include "../Component/PlayerComponent.h"
#include "../Component/EnemyComponent.h"
#include "../Component/ColliderComponent.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

// ============================================================
//  Update
// ============================================================

void EnemyCollisionSystem::Update(No::Registry& registry, float deltaTime) {
	// プレイヤーを取得
	auto playerView = registry.View<PlayerComponent, PlayerTag, SphereColliderComponent, HealthComponent, DeathFlag>();
	if (playerView.Empty()) return;

	PlayerComponent* player = nullptr;
	SphereColliderComponent* playerCollider = nullptr;
	HealthComponent* playerHealth = nullptr;
	DeathFlag* playerDeath = nullptr;

	{
		// Empty() チェック済みのため begin() は必ず有効
		auto it = playerView.begin();
		player = registry.GetComponent<PlayerComponent>(*it);
		playerCollider = registry.GetComponent<SphereColliderComponent>(*it);
		playerHealth = registry.GetComponent<HealthComponent>(*it);
		playerDeath = registry.GetComponent<DeathFlag>(*it);
	}

	if (!player || !playerCollider || !playerHealth || !playerDeath) return;
	// すでに死亡済みのプレイヤーには何もしない
	if (playerDeath->isDead) return;

	// 一時無敵タイマー更新
	if (playerHealth->invincibilityTimer > 0.0f) {
		playerHealth->invincibilityTimer -= deltaTime;
		if (playerHealth->invincibilityTimer < 0.0f) playerHealth->invincibilityTimer = 0.0f;
	}

	// デバッグ UI
#ifdef USE_IMGUI
	DebugUI(playerHealth);
#endif

	// CollisionSystemが設定した衝突フラグをチェック
	if (playerCollider->isCollied && playerCollider->colliedWith == kEnemy) {
		auto enemyEntity = playerCollider->colliedEntity;

		// 敵が有効か確認
		if (registry.Has<DeathFlag>(enemyEntity)) {
			auto* enemyDeath = registry.GetComponent<DeathFlag>(enemyEntity);

			if (!enemyDeath->isDead) {
				// スポーニング状態の敵は衝突判定を行わない
				if (registry.Has<EnemyComponent>(enemyEntity)) {
					auto* ec = registry.GetComponent<EnemyComponent>(enemyEntity);
					if (ec->isSpawning) return;
				}

				// 敵を死亡させる
				enemyDeath->isDead = true;

				// 敵撃破カウント増加
				if (onEnemyKilled_) {
					onEnemyKilled_();
				}

				// プレイヤーにダメージを与える
				bool died = playerHealth->TakeDamage(1);
				if (died && !playerHealth->isInvincible) {
					playerDeath->isDead = true;
				}
			}
		}
	}
}

#ifdef USE_IMGUI
void EnemyCollisionSystem::DebugUI(HealthComponent* playerHealth) {
	ImGui::Begin("Player Health");

	ImGui::Text("HP: %d / %d", playerHealth->currentHp, playerHealth->maxHp);

	// HPバーを色付きで表示
	float hpRatio = (playerHealth->maxHp > 0)
		? static_cast<float>(playerHealth->currentHp) / static_cast<float>(playerHealth->maxHp)
		: 0.0f;
	ImVec4 barColor = (hpRatio > 0.5f) ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f)
		: (hpRatio > 0.25f) ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f)
		: ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barColor);
	ImGui::ProgressBar(hpRatio, ImVec2(-1, 0), "HP");
	ImGui::PopStyleColor();

	ImGui::Separator();
	// デバッグ用無敵フラグ
	ImGui::Checkbox("無敵モード (Debug)", &playerHealth->isInvincible);

	if (playerHealth->invincibilityTimer > 0.0f) {
		ImGui::Text("一時無敵: %.2f 秒", playerHealth->invincibilityTimer);
	}

	ImGui::Separator();
	// HP 直接操作（デバッグ用）
	ImGui::DragInt("現在HP", &playerHealth->currentHp, 1, 0, playerHealth->maxHp);
	ImGui::DragInt("最大HP", &playerHealth->maxHp, 1, 1, 99);
	if (ImGui::Button("HP 全回復")) {
		playerHealth->currentHp = playerHealth->maxHp;
	}

	ImGui::End();
}
#endif