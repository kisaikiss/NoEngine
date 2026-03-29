#include "stdafx.h"
#include "EnemyShootSystem.h"
#include "application/CommentBout/Component/Enemy/EnemyComponent.h"
#include "application/CommentBout/Component/Enemy/EnemyShooterComponent.h"
#include "application/CommentBout/Component/Enemy/EnemyBulletComponent.h"
#include "application/CommentBout/Component/Enemy/BossComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Component/LifetimeComponent.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/Collision/Component/Collider3DComponent.h"
#include "application/CommentBout/Collision/Component/ProjectedColliderComponent.h"
#include "application/CommentBout/Collision/Utility/CoordinateConverter.h"
#include "application/CommentBout/Utility/EnemyVisibilityUtility.h"
#include "application/CommentBout/Utility/CBCollisionMask.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Utility/CBGameAudio.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Functions/ECS/Component/MaterialComponent.h"
#include <algorithm>

namespace {
No::Vector3 NormalizeOrDefault(const No::Vector3& v, const No::Vector3& fallback) {
	if (v.LengthSquared() <= 0.000001f) {
		return fallback;
	}
	return v.Normalize();
}

No::Vector3 ComputeAimDirection(
	No::Registry& registry,
	No::TransformComponent& fromTransform,
	No::CameraComponent& camera,
	No::TransformComponent& cameraTransform,
	float depthFromCamera,
	No::Vector3* outAimTarget
) {
	// ここで求めるのは「照準点」までの進行方向のみ。
	// 被弾判定は EnemyBulletHitSystem 側の投影+ゲート判定で処理する。
	auto playerView = registry.View<CBPlayerTag, No::Transform2DComponent>();
	auto playerIt = playerView.begin();
	if (playerIt == playerView.end()) {
		return No::Vector3::FORWARD;
	}
	auto* playerTransform2D = registry.GetComponent<No::Transform2DComponent>(*playerIt);
	if (!playerTransform2D) {
		return No::Vector3::FORWARD;
	}
	auto* mainWindow = NoEngine::GraphicsCore::gWindowManager.GetMainWindow();
	if (!mainWindow) {
		return No::Vector3::FORWARD;
	}

	No::Vector3 targetWorld = CommentBoutCollision::CoordinateConverter::ScreenToWorldNearCamera(
		playerTransform2D->translate,
		depthFromCamera,
		camera,
		cameraTransform,
		mainWindow->GetWindowSize()
	);
	if (outAimTarget) {
		*outAimTarget = targetWorld;
	}
	return NormalizeOrDefault(targetWorld - fromTransform.GetWorldPosition(), No::Vector3::FORWARD);
}

void SpawnEnemyBullet(
	No::Registry& registry,
	No::Entity owner,
	No::TransformComponent& fromTransform,
	const No::Vector3& direction,
	float bulletSpeed,
	int bulletDamage,
	float lifetimeSec,
	float bulletRadiusMultiplier,
	const No::Vector3& bulletLocalOffset,
	const GameResourceComponent* gameResource
) {
	auto bullet = registry.GenerateEntity();
	registry.AddComponent<CBEnemyBulletTag>(bullet);

	auto* t = registry.AddComponent<No::TransformComponent>(bullet);
	t->translate = fromTransform.GetWorldPosition() + direction * 0.7f;
	t->scale = { 0.25f, 0.25f, 0.25f };

	auto* mesh = registry.AddComponent<No::MeshComponent>(bullet);
	auto* material = registry.AddComponent<No::MaterialComponent>(bullet);
	if (gameResource) {
		if (const auto* bulletModel = FindGameModel(*gameResource, CommentBoutResourceKey::kEnemyBulletModel)) {
			No::ModelLoader::GetModel(bulletModel->assetName, mesh);
			material->materials = No::ModelLoader::GetMaterial(bulletModel->assetName);
		}
	}
	material->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	material->psoName = L"Renderer : Default PSO";
	material->psoId = NoEngine::Render::GetPSOID(material->psoName);
	material->rootSigId = NoEngine::Render::GetRootSignatureID(material->psoName);

	auto* bulletComp = registry.AddComponent<EnemyBulletComponent>(bullet);
	bulletComp->velocity = direction * std::max(0.01f, bulletSpeed);
	bulletComp->damage = std::max(1, bulletDamage);
	bulletComp->owner = owner;

	auto* collider = registry.AddComponent<CommentBoutCollision::Collider3DComponent>(bullet);
	collider->shapeType = CommentBoutCollision::ShapeType3D::Sphere;
	collider->useScaleAsRadius = true;
	collider->radiusMultiplier = bulletRadiusMultiplier;
	collider->localOffset = bulletLocalOffset;
	collider->collisionLayer = CommentBout::CollisionLayer::CBEnemyBullet;
	collider->collisionMask = CommentBout::CollisionMask::CBEnemyBullet;

	auto* projected = registry.AddComponent<CommentBoutCollision::ProjectedColliderComponent>(bullet);
	projected->source3DEntity = bullet;

	auto* life = registry.AddComponent<LifetimeComponent>(bullet);
	life->remainingTime = std::max(0.1f, lifetimeSec);
}
}

void EnemyShootSystem::Update(No::Registry& registry, float deltaTime)
{
	GameResourceComponent* gameResource = nullptr;
	auto resourceView = registry.View<CBGameResourceTag, GameResourceComponent>();
	for (auto entity : resourceView) {
		gameResource = registry.GetComponent<GameResourceComponent>(entity);
		if (gameResource) {
			break;
		}
	}

	auto cameraView = registry.View<No::ActiveCameraTag, No::CameraComponent, No::TransformComponent>();
	auto cameraIt = cameraView.begin();
	if (cameraIt == cameraView.end()) {
		return;
	}
	auto* camera = registry.GetComponent<No::CameraComponent>(*cameraIt);
	auto* cameraTransform = registry.GetComponent<No::TransformComponent>(*cameraIt);
	if (!camera || !cameraTransform) {
		return;
	}

	auto* mainWindow = NoEngine::GraphicsCore::gWindowManager.GetMainWindow();
	if (!mainWindow) {
		return;
	}
	const NoEngine::WindowSize windowSize = mainWindow->GetWindowSize();

	auto shooterView = registry.View<CBRailEnemyTag, EnemyShooterComponent, EnemyComponent, HealthComponent, No::TransformComponent>();
	for (auto entity : shooterView) {
		if (registry.Has<CBBossTag>(entity)) {
			continue;
		}
		auto* shooter = registry.GetComponent<EnemyShooterComponent>(entity);
		auto* health = registry.GetComponent<HealthComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		if (!shooter || !health || !transform || health->isDead || health->hp <= 0) {
			continue;
		}

		shooter->shootCooldown -= deltaTime;
		if (shooter->shootCooldown > 0.0f) {
			continue;
		}

		if (!CommentBoutVisibility::IsWorldPositionVisibleOnScreen(transform->GetWorldPosition(), *camera, windowSize)) {
			continue;
		}

		No::Vector3 aimTarget{};
		No::Vector3 dir = ComputeAimDirection(registry, *transform, *camera, *cameraTransform, shooter->targetDepthFromCamera, &aimTarget);
		if (shooter->shootDistanceMax > 0.0f) {
			const float distanceToPlayer = (aimTarget - transform->GetWorldPosition()).Length();
			if (distanceToPlayer > shooter->shootDistanceMax) {
				continue;
			}
		}

		shooter->shootCooldown = std::max(0.05f, shooter->shootInterval);
		SpawnEnemyBullet(registry, entity, *transform, dir, shooter->bulletSpeed, shooter->bulletDamage, shooter->bulletLifetime,
			shooter->bulletColliderRadiusMultiplier, shooter->bulletColliderLocalOffset, gameResource);
		CommentBout::GameAudio::PlaySEClip(CommentBoutResourceKey::kSEEnemyShot);
	}

	auto bossView = registry.View<CBBossTag, BossComponent, EnemyShooterComponent, HealthComponent, No::TransformComponent>();
	for (auto entity : bossView) {
		auto* boss = registry.GetComponent<BossComponent>(entity);
		auto* shooter = registry.GetComponent<EnemyShooterComponent>(entity);
		auto* health = registry.GetComponent<HealthComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		if (!boss || !shooter || !health || !transform || health->isDead || health->hp <= 0) {
			continue;
		}
		if (boss->phase != BossPhase::Shoot) {
			continue;
		}

		if (!CommentBoutVisibility::IsWorldPositionVisibleOnScreen(transform->GetWorldPosition(), *camera, windowSize)) {
			continue;
		}

		No::Vector3 aimTarget{};
		No::Vector3 dir = ComputeAimDirection(registry, *transform, *camera, *cameraTransform, shooter->targetDepthFromCamera, &aimTarget);
		if (shooter->shootDistanceMax > 0.0f) {
			const float distanceToPlayer = (aimTarget - transform->GetWorldPosition()).Length();
			if (distanceToPlayer > shooter->shootDistanceMax) {
				continue;
			}
		}

		const int prevShots = boss->shotsSpawnedInBurst;
		while (boss->shotsSpawnedInBurst < boss->shotsFiredInBurst) {
			SpawnEnemyBullet(registry, entity, *transform, dir, shooter->bulletSpeed, shooter->bulletDamage, shooter->bulletLifetime,
				shooter->bulletColliderRadiusMultiplier, shooter->bulletColliderLocalOffset, gameResource);
			boss->shotsSpawnedInBurst += 1;
		}
		if (boss->shotsSpawnedInBurst > prevShots) {
			CommentBout::GameAudio::PlaySEClip(CommentBoutResourceKey::kSEBossShot);
		}
	}

	auto bulletView = registry.View<CBEnemyBulletTag, EnemyBulletComponent, No::TransformComponent>();
	for (auto bulletEntity : bulletView) {
		auto* bullet = registry.GetComponent<EnemyBulletComponent>(bulletEntity);
		auto* transform = registry.GetComponent<No::TransformComponent>(bulletEntity);
		if (!bullet || !transform) {
			continue;
		}
		transform->translate += bullet->velocity * deltaTime;
	}
}
