#include "stdafx.h"
#include "EnemySpawnSystem.h"
#include "application/CommentBout/Component/SpawnEnemyRequestComponent.h"
#include "application/CommentBout/Component/RailCameraComponent.h"
#include "application/CommentBout/Component/EnemyComponent.h"
#include "application/CommentBout/Component/EnemyShooterComponent.h"
#include "application/CommentBout/Component/EnemyRewardSourceComponent.h"
#include "application/CommentBout/Component/BossComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Data/EnemyTypePresetIO.h"
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

void SpawnRailEnemies(No::Registry& registry, const RailEnemySpawnEventParams& params) {
	static EnemyTypePresetMap presetMap;
	static bool presetLoaded = false;
	if (!presetLoaded) {
		LoadEnemyTypePresetMap(presetMap);
		presetLoaded = true;
	}
	const EnemyTypePreset preset = GetEnemyTypePresetOrDefault(presetMap, params.enemyType);

	const No::Vector3 direction = NormalizeOrDefault(params.moveDirection, No::Vector3(0.0f, 0.0f, -1.0f));
	const int spawnCount = std::max(1, params.count);

	for (int i = 0; i < spawnCount; ++i) {
		auto enemyEntity = registry.GenerateEntity();
		registry.AddComponent<CBRailEnemyTag>(enemyEntity);

		auto* transform = registry.AddComponent<No::TransformComponent>(enemyEntity);
		transform->translate = params.spawnPosition + direction * (params.spawnSpacing * static_cast<float>(i));
		transform->scale = { preset.modelScale, preset.modelScale, preset.modelScale };

		auto* mesh = registry.AddComponent<No::MeshComponent>(enemyEntity);
		auto* material = registry.AddComponent<No::MaterialComponent>(enemyEntity);
		No::ModelLoader::LoadModel("commentbout_rail_enemy_cube", "resources/game/td_3105/Model/cube/cube.obj", mesh);
		material->materials = No::ModelLoader::GetMaterial("commentbout_rail_enemy_cube");
		material->color = { 1.0f, 1.0f, 1.0f, 1.0f };
		material->psoName = L"Renderer : Default PSO";
		material->psoId = NoEngine::Render::GetPSOID(material->psoName);
		material->rootSigId = NoEngine::Render::GetRootSignatureID(material->psoName);

		auto* enemy = registry.AddComponent<EnemyComponent>(enemyEntity);
		enemy->maxHp = std::max(1, preset.minHp);
		enemy->hp = enemy->maxHp;
		enemy->moveSpeed = std::max(0.0f, params.moveSpeed);
		enemy->moveDirection = direction;
		enemy->groupId = params.spawnGroupId;

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
			std::max(0.01f, preset.baseColliderBox.x),
			std::max(0.01f, preset.baseColliderBox.y),
			std::max(0.01f, preset.baseColliderBox.z)
		};
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

		SpawnRailEnemies(registry, request->params);
		consumed.push_back(entity);
	}

	for (auto entity : consumed) {
		registry.DestroyEntity(entity);
	}
}
