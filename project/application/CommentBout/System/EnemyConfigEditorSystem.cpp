#include "stdafx.h"
#include "EnemyConfigEditorSystem.h"
#include "application/CommentBout/Data/EnemyDataIO.h"
#include "application/CommentBout/Component/EnemyComponent.h"
#include "application/CommentBout/Component/EnemyShooterComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Component/EnemyRewardSourceComponent.h"
#include "application/CommentBout/Collision/Component/Collider3DComponent.h"
#include "application/CommentBout/GameTag.h"
#include <algorithm>

void EnemyConfigEditorSystem::ApplyToAliveEnemies(No::Registry& registry)
{
	auto view = registry.View<CBRailEnemyTag, EnemyComponent, HealthComponent, No::TransformComponent>();
	for (auto entity : view) {
		auto* enemy = registry.GetComponent<EnemyComponent>(entity);
		auto* health = registry.GetComponent<HealthComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		if (!enemy || !health || !transform) continue;

		RailEnemyType enemyType = RailEnemyType::MoveOnly;
		if (registry.Has<CBBossTag>(entity))            enemyType = RailEnemyType::Boss;
		else if (registry.Has<EnemyShooterComponent>(entity)) enemyType = RailEnemyType::MoveAndShoot;

		const EnemyConfig cfg = EnemyDataIO::GetOrDefault(presets_, enemyType);

		transform->scale = { cfg.modelScale, cfg.modelScale, cfg.modelScale };
		enemy->despawnBehindDistance = std::max(0.0f, cfg.despawnBehindDistance);
		enemy->maxHp = std::max(1, cfg.minHp);
		enemy->hp = std::min(enemy->hp, enemy->maxHp);
		health->maxHp = enemy->maxHp;
		health->hp = std::min(health->hp, health->maxHp);

		if (auto* rs = registry.GetComponent<EnemyRewardSourceComponent>(entity))
			rs->worldSizeForReward = cfg.modelScale;

		if (auto* col = registry.GetComponent<CommentBoutCollision::Collider3DComponent>(entity)) {
			col->shapeType = CommentBoutCollision::ShapeType3D::Box;
			col->useScaleAsBox = true;
			col->boxSizeMultiplier = {
				std::max(0.01f, cfg.enemyCollider.boxSizeMultiplier.x),
				std::max(0.01f, cfg.enemyCollider.boxSizeMultiplier.y),
				std::max(0.01f, cfg.enemyCollider.boxSizeMultiplier.z)
			};
			col->localOffset = cfg.enemyCollider.localOffset3D;
		}

		if (auto* shooter = registry.GetComponent<EnemyShooterComponent>(entity)) {
			shooter->shootInterval = std::max(0.05f, cfg.shootInterval);
			shooter->bulletSpeed = std::max(0.1f, cfg.bulletSpeed);
			shooter->bulletDamage = std::max(1, cfg.bulletDamage);
			shooter->targetDepthFromCamera = std::max(0.1f, cfg.targetDepthFromCamera);
			shooter->bulletLifetime = std::max(0.1f, cfg.bulletLifetime);
			shooter->shootDistanceMax = std::max(0.0f, cfg.shootDistanceMax);
		}
	}
}

void EnemyConfigEditorSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);


	if (!loaded_) {
		presets_ = EnemyDataIO::Load();
		loaded_ = true;
	}

#ifdef USE_IMGUI
	ImGui::Begin("敵プリセット設定");

	if (ImGui::Button("JSONから再読み込み")) {
		presets_ = EnemyDataIO::Load();
		ApplyToAliveEnemies(registry);
	}
	ImGui::SameLine();
	if (ImGui::Button("JSONへ保存")) {
		EnemyDataIO::Save(presets_);
	}

	ImGui::Separator();

	auto drawType = [&](RailEnemyType type, const char* label) {
		EnemyConfig cfg = EnemyDataIO::GetOrDefault(presets_, type);
		if (!ImGui::TreeNode(label)) return;

		bool changed = false;

		ImGui::SeparatorText("外観");
		changed |= ImGui::DragFloat("モデルスケール", &cfg.modelScale, 0.01f, 0.1f, 10.0f);

		ImGui::SeparatorText("当たり判定");
		changed |= ImGui::DragFloat3("コライダーサイズ(Box)", &cfg.enemyCollider.boxSizeMultiplier.x, 0.01f, 0.01f, 20.0f);
		changed |= ImGui::DragFloat3("コライダーオフセット", &cfg.enemyCollider.localOffset3D.x, 0.01f, -10.0f, 10.0f);

		ImGui::SeparatorText("ステータス");
		changed |= ImGui::DragInt("HP", &cfg.minHp, 1, 1, 9999);
		changed |= ImGui::DragFloat("デスポーン距離", &cfg.despawnBehindDistance, 0.5f, 0.0f, 200.0f);

		ImGui::SeparatorText("射撃");
		changed |= ImGui::DragFloat("射撃間隔(秒)", &cfg.shootInterval, 0.01f, 0.05f, 10.0f);
		changed |= ImGui::DragFloat("弾速", &cfg.bulletSpeed, 0.05f, 0.1f, 100.0f);
		changed |= ImGui::DragInt("弾ダメージ", &cfg.bulletDamage, 1, 1, 999);
		changed |= ImGui::DragFloat("弾の寿命(秒)", &cfg.bulletLifetime, 0.05f, 0.1f, 30.0f);
		changed |= ImGui::DragFloat("カメラからの射撃深度", &cfg.targetDepthFromCamera, 0.01f, 0.1f, 20.0f);
		changed |= ImGui::DragFloat("射撃最大距離", &cfg.shootDistanceMax, 0.05f, 0.0f, 200.0f);

		presets_[type] = cfg;

		if (changed) {
			ApplyToAliveEnemies(registry);
		}

		ImGui::TreePop();
		};

	drawType(RailEnemyType::MoveOnly, "通常敵 (MoveOnly)");
	drawType(RailEnemyType::MoveAndShoot, "射撃敵 (MoveAndShoot)");
	drawType(RailEnemyType::Boss, "ボス (Boss)");

	ImGui::End();
#else
	static_cast<void>(registry);
#endif
}
