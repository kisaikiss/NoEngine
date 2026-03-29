#include "stdafx.h"
#include "BossBehaviorSystem.h"
#include "application/CommentBout/Component/Enemy/BossComponent.h"
#include "application/CommentBout/Component/Enemy/EnemyComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Component/RailCameraComponent.h"
#include "application/CommentBout/GameTag.h"
#include <cmath>

namespace {
No::Vector3 NormalizeOrDefault(const No::Vector3& v, const No::Vector3& fallback) {
	if (v.LengthSquared() <= 0.000001f) {
		return fallback;
	}
	return v.Normalize();
}

No::Vector3 ComputeCameraRelativeOrigin(No::TransformComponent& cameraTransform, const No::Vector3& offsetLocal) {
	No::Matrix4x4 cameraWorld = cameraTransform.MakeAffineMatrix4x4();
	No::Vector3 right = NormalizeOrDefault(cameraWorld.TransformNormal(No::Vector3::RIGHT), No::Vector3::RIGHT);
	No::Vector3 up = NormalizeOrDefault(cameraWorld.TransformNormal(No::Vector3::UP), No::Vector3::UP);
	No::Vector3 forward = NormalizeOrDefault(cameraWorld.TransformNormal(No::Vector3::FORWARD), No::Vector3::FORWARD);
	const No::Vector3 cameraPos = cameraTransform.GetWorldPosition();
	return cameraPos + right * offsetLocal.x + up * offsetLocal.y + forward * offsetLocal.z;
}

void FaceToCamera(No::TransformComponent& bossTransform, No::TransformComponent& cameraTransform) {
	No::Vector3 toCamera = cameraTransform.GetWorldPosition() - bossTransform.GetWorldPosition();
	toCamera = NormalizeOrDefault(toCamera, No::Vector3::FORWARD);
	bossTransform.rotation.LookRotation(toCamera, No::Vector3::UP);
}
}

void BossBehaviorSystem::Update(No::Registry& registry, float deltaTime)
{
	auto cameraView = registry.View<RailCameraComponent, No::TransformComponent>();
	auto cameraIt = cameraView.begin();
	if (cameraIt == cameraView.end()) {
		return;
	}
	auto* cameraTransform = registry.GetComponent<No::TransformComponent>(*cameraIt);
	if (!cameraTransform) {
		return;
	}

	constexpr float kTwoPi = 6.28318530718f;
	auto bossView = registry.View<CBBossTag, BossComponent, EnemyComponent, HealthComponent, No::TransformComponent>();
	for (auto entity : bossView) {
		auto* boss = registry.GetComponent<BossComponent>(entity);
		auto* enemy = registry.GetComponent<EnemyComponent>(entity);
		auto* health = registry.GetComponent<HealthComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		if (!boss || !enemy || !health || !transform) {
			continue;
		}
		if (health->isDead || health->hp <= 0) {
			continue;
		}

		boss->worldOrigin = ComputeCameraRelativeOrigin(*cameraTransform, boss->behavior.offsetLocal);

		switch (boss->phase) {
		case BossPhase::Move:
		{
			boss->phaseTime += deltaTime;
			const float period = std::max(0.1f, boss->behavior.figure8Period);
			const float t = boss->phaseTime / period;
			const float angle = t * kTwoPi;
			const float x = boss->behavior.figure8Amplitude.x * std::sinf(angle);
			const float y = boss->behavior.figure8Amplitude.y * std::sinf(angle * 2.0f);
			transform->translate = boss->worldOrigin + No::Vector3(x, y, 0.0f);

			if (boss->phaseTime >= period) {
				boss->phase = BossPhase::Stop;
				boss->phaseTime = 0.0f;
			}
			break;
		}
		case BossPhase::Stop:
		{
			boss->phaseTime += deltaTime;
			transform->translate = boss->worldOrigin;
			if (boss->phaseTime >= std::max(0.0f, boss->behavior.stopDuration)) {
				boss->phase = BossPhase::Shoot;
				boss->phaseTime = 0.0f;
				boss->shotTimer = 0.0f;
				boss->shotsFiredInBurst = 0;
				boss->shotsSpawnedInBurst = 0;
			}
			break;
		}
		case BossPhase::Shoot:
		{
			boss->shotTimer += deltaTime;
			transform->translate = boss->worldOrigin;
			const float interval = std::max(0.05f, boss->behavior.burstShotInterval);
			if (boss->shotTimer >= interval && boss->shotsFiredInBurst < 3) {
				boss->shotTimer = 0.0f;
				boss->shotsFiredInBurst += 1;
			}
			if (boss->shotsFiredInBurst >= 3) {
				boss->phase = BossPhase::Move;
				boss->phaseTime = 0.0f;
			}
			break;
		}
		default:
			break;
		}

		FaceToCamera(*transform, *cameraTransform);
		enemy->moveDirection = NormalizeOrDefault(cameraTransform->GetWorldPosition() - transform->GetWorldPosition(), No::Vector3::FORWARD);
	}
}
