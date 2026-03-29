#include "stdafx.h"
#include "EnemySpawnSystem.h"
#include "application/CommentBout/Component/Enemy/SpawnEnemyRequestComponent.h"
#include "application/CommentBout/Component/RailCameraComponent.h"
#include "application/CommentBout/Component/Enemy/EnemyComponent.h"
#include "application/CommentBout/Component/Enemy/EnemyShooterComponent.h"
#include "application/CommentBout/Component/Enemy/EnemyRewardSourceComponent.h"
#include "application/CommentBout/Component/Enemy/BossComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/Data/EnemyConfig.h"
#include "application/CommentBout/Data/EnemyDataIO.h"
#include "application/CommentBout/Utility/CBCollisionMask.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Collision/Component/Collider3DComponent.h"
#include "application/CommentBout/Collision/Component/ProjectedColliderComponent.h"
#include "engine/Functions/ECS/Component/MaterialComponent.h"
#include <algorithm>

namespace {
No::Vector3 NormalizeOrDefault(const No::Vector3& v, const No::Vector3& fallback) {
	if (v.LengthSquared() <= 0.000001f) {
		return fallback;
	}
	return v.Normalize();
}

void SpawnRailEnemies(No::Registry& registry, const RailEnemySpawnEventParams& params, const GameResourceComponent* gameResource) {
	EnemyConfigMap configMap = EnemyDataIO::Load();
	const EnemyConfig preset = EnemyDataIO::GetOrDefault(configMap, params.enemyType);

	const No::Vector3 direction = NormalizeOrDefault(params.moveDirection, No::Vector3(0.0f, 0.0f, -1.0f));
	const int spawnCount = std::max(1, params.count);

	for (int i = 0; i < spawnCount; ++i) {
		auto enemyEntity = registry.GenerateEntity();
		registry.AddComponent<CBRailEnemyTag>(enemyEntity);

		auto* transform = registry.AddComponent<No::TransformComponent>(enemyEntity);
		transform->translate = params.spawnPosition + params.spawnOffset * static_cast<float>(i);
		transform->scale = { preset.modelScale, preset.modelScale, preset.modelScale };

		auto* mesh = registry.AddComponent<No::MeshComponent>(enemyEntity);
		auto* material = registry.AddComponent<No::MaterialComponent>(enemyEntity);
		if (gameResource) {
			const char* modelKey = CommentBoutResourceKey::kEnemyModel;
			if (params.enemyType == RailEnemyType::MoveAndShoot) {
				modelKey = CommentBoutResourceKey::kShootEnemyModel;
			} else if (params.enemyType == RailEnemyType::Boss) {
				modelKey = CommentBoutResourceKey::kBossModel;
			}
			if (const auto* model = FindGameModel(*gameResource, modelKey)) {
				No::ModelLoader::GetModel(model->assetName, mesh);
				material->materials = No::ModelLoader::GetMaterial(model->assetName);
			}
		}
		material->color = { 1.0f, 1.0f, 1.0f, 1.0f };
		material->psoName = L"Renderer : Default PSO";
		material->psoId = NoEngine::Render::GetPSOID(material->psoName);
		material->rootSigId = NoEngine::Render::GetRootSignatureID(material->psoName);

		auto* enemy = registry.AddComponent<EnemyComponent>(enemyEntity);
		enemy->maxHp = std::max(1, preset.minHp);
		enemy->hp = enemy->maxHp;
		enemy->moveSpeed = std::max(0.0f, params.moveSpeed);
		enemy->moveDirection = direction;
		enemy->despawnBehindDistance = std::max(0.0f, preset.despawnBehindDistance);
		enemy->groupId = params.spawnGroupId;
		enemy->removeReason = EnemyRemoveReason::None;

		auto* rewardSource = registry.AddComponent<EnemyRewardSourceComponent>(enemyEntity);
		rewardSource->worldSizeForReward = preset.modelScale;
		rewardSource->spawned = false;

		auto* commonHealth = registry.AddComponent<HealthComponent>(enemyEntity);
		commonHealth->hp = enemy->hp;
		commonHealth->maxHp = enemy->maxHp;
		commonHealth->isDead = false;
		commonHealth->lastDamageTaken = 0;

		auto* collider3D = registry.AddComponent<CommentBoutCollision::Collider3DComponent>(enemyEntity);
		collider3D->shapeType = CommentBoutCollision::ShapeType3D::Box;
		collider3D->useScaleAsBox = true;
		collider3D->boxSizeMultiplier = {
			std::max(0.01f, preset.enemyCollider.boxSizeMultiplier.x),
			std::max(0.01f, preset.enemyCollider.boxSizeMultiplier.y),
			std::max(0.01f, preset.enemyCollider.boxSizeMultiplier.z)
		};
		collider3D->localOffset = preset.enemyCollider.localOffset3D;
		collider3D->collisionLayer = CommentBout::CollisionLayer::CBEnemy;
		collider3D->collisionMask = CommentBout::CollisionLayer::CBPlayerAttack;

		auto* projected = registry.AddComponent<CommentBoutCollision::ProjectedColliderComponent>(enemyEntity);
		projected->source3DEntity = enemyEntity;

		switch (params.enemyType) {
		case RailEnemyType::MoveAndShoot:
		{
			auto* shooter = registry.AddComponent<EnemyShooterComponent>(enemyEntity);
			shooter->shootInterval = std::max(0.05f, preset.shootInterval);
			shooter->bulletSpeed = std::max(0.1f, preset.bulletSpeed);
			shooter->bulletDamage = std::max(1, preset.bulletDamage);
			shooter->targetDepthFromCamera = std::max(0.1f, preset.targetDepthFromCamera);
			shooter->bulletLifetime = std::max(0.1f, preset.bulletLifetime);
			shooter->shootDistanceMax = std::max(0.0f, preset.shootDistanceMax);
			shooter->bulletModelScale               = std::max(0.01f, preset.bulletModelScale);
			shooter->bulletColliderRadiusMultiplier = std::max(0.01f, preset.bulletCollider.radiusMultiplier);
			shooter->bulletColliderLocalOffset      = preset.bulletCollider.localOffset3D;
			break;
		}
		case RailEnemyType::Boss:
		{
			registry.AddComponent<CBBossTag>(enemyEntity);
			auto* boss = registry.AddComponent<BossComponent>(enemyEntity);
			boss->behavior = params.boss;
			auto* shooter = registry.AddComponent<EnemyShooterComponent>(enemyEntity);
			shooter->shootInterval = std::max(0.05f, preset.shootInterval);
			shooter->bulletSpeed = std::max(0.1f, preset.bulletSpeed);
			shooter->bulletDamage = std::max(1, preset.bulletDamage);
			shooter->targetDepthFromCamera = std::max(0.1f, preset.targetDepthFromCamera);
			shooter->bulletLifetime = std::max(0.1f, preset.bulletLifetime);
			shooter->shootDistanceMax = std::max(0.0f, preset.shootDistanceMax);
			shooter->bulletModelScale               = std::max(0.01f, preset.bulletModelScale);
			shooter->bulletColliderRadiusMultiplier = std::max(0.01f, preset.bulletCollider.radiusMultiplier);
			shooter->bulletColliderLocalOffset      = preset.bulletCollider.localOffset3D;
			break;
		}
		case RailEnemyType::MoveOnly:
		default:
			break;
		}
	}
}
}

void EnemySpawnSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);

	GameResourceComponent* gameResource = nullptr;
	auto resourceView = registry.View<CBGameResourceTag, GameResourceComponent>();
	for (auto entity : resourceView) {
		gameResource = registry.GetComponent<GameResourceComponent>(entity);
		if (gameResource) {
			break;
		}
	}

	std::vector<No::Entity> consumed;
	auto view = registry.View<SpawnEnemyRequestComponent>();
	for (auto entity : view) {
		auto* request = registry.GetComponent<SpawnEnemyRequestComponent>(entity);
		if (!request) {
			consumed.push_back(entity);
			continue;
		}
		if (request->params.count <= 0) {
			consumed.push_back(entity);
			continue;
		}

		SpawnRailEnemies(registry, request->params, gameResource);
		consumed.push_back(entity);
	}

	for (auto entity : consumed) {
		registry.DestroyEntity(entity);
	}
}
