#include "stdafx.h"
#include "EnemyMoveSystem.h"
#include "application/CommentBout/Component/EnemyComponent.h"
#include "application/CommentBout/Component/EnemyRewardSourceComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Utility/EnemyVisibilityUtility.h"
#include "application/CommentBout/GameTag.h"

namespace {
No::Vector3 NormalizeOrDefault(const No::Vector3& v, const No::Vector3& fallback) {
	if (v.LengthSquared() <= 0.000001f) {
		return fallback;
	}
	return v.Normalize();
}
}

void EnemyMoveSystem::Update(No::Registry& registry, float deltaTime)
{
	No::TransformComponent* cameraTransform = nullptr;
	auto cameraView = registry.View<No::ActiveCameraTag, No::TransformComponent>();
	auto cameraIt = cameraView.begin();
	if (cameraIt != cameraView.end()) {
		cameraTransform = registry.GetComponent<No::TransformComponent>(*cameraIt);
	}

	auto view = registry.View<CBRailEnemyTag, EnemyComponent, HealthComponent, No::TransformComponent>();
	for (auto entity : view) {
		auto* enemy = registry.GetComponent<EnemyComponent>(entity);
		auto* health = registry.GetComponent<HealthComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		if (!enemy || !health || !transform) {
			continue;
		}
		if (health->isDead || health->hp <= 0) {
			continue;
		}

		// 敵移動は種類に依存せず共通で適用する（通常敵/射撃敵/ボス）。
		const No::Vector3 moveDir = NormalizeOrDefault(enemy->moveDirection, No::Vector3(0.0f, 0.0f, -1.0f));
		transform->translate += moveDir * enemy->moveSpeed * deltaTime;

		//敵は移動方向の方を向く
		if (moveDir.LengthSquared() > 0.000001f) {
			No::Vector3 forward = transform->rotation.RotateVector(No::Vector3::FORWARD);
			forward.y = 0.0f; // 水平方向のみで向きを決める
			forward = NormalizeOrDefault(forward, No::Vector3(0.0f, 0.0f, -1.0f));
			const float angle = std::atan2(moveDir.x, moveDir.z) - std::atan2(forward.x, forward.z);
			No::Quaternion rotY;
			rotY.FromAxisAngle(No::Vector3::UP, angle);
			transform->rotation = rotY * transform->rotation;
		}
		

		if (cameraTransform && enemy->despawnBehindDistance > 0.0f) {
			const float forwardDistance = CommentBoutVisibility::ComputeForwardDistanceFromCamera(*transform, *cameraTransform);
			if (forwardDistance < -enemy->despawnBehindDistance) {
				// 自機より十分後方へ流れた敵は「自然消滅」として削除する。
				// 撃破ではないため、報酬オーブは生成しない。
				enemy->removeReason = EnemyRemoveReason::NaturalDespawn;
				if (auto* rewardSource = registry.GetComponent<EnemyRewardSourceComponent>(entity)) {
					rewardSource->spawned = true;
				}
				registry.DestroyEntity(entity);
			}
		}
	}
}
