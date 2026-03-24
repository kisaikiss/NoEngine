#include "stdafx.h"
#include "EnemySystem.h"

#include "application/CommentBout/Component/RailCameraComponent.h"
#include "application/CommentBout/Component/EnemyComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
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

void SpawnRailEnemiesInternal(No::Registry& registry, const RailEnemySpawnEventParams& params) {
	const No::Vector3 direction = NormalizeOrDefault(params.moveDirection, No::Vector3(0.0f, 0.0f, -1.0f));
	const int spawnCount = std::max(1, params.count);

	for (int i = 0; i < spawnCount; ++i) {
		auto enemyEntity = registry.GenerateEntity();
		registry.AddComponent<CBRailEnemyTag>(enemyEntity);

		auto* transform = registry.AddComponent<No::TransformComponent>(enemyEntity);
		transform->translate = params.spawnPosition + direction * (params.spawnSpacing * static_cast<float>(i));
		transform->scale = { 0.7f, 0.7f, 0.7f };

		auto* mesh = registry.AddComponent<No::MeshComponent>(enemyEntity);
		auto* material = registry.AddComponent<No::MaterialComponent>(enemyEntity);
		No::ModelLoader::LoadModel("commentbout_rail_enemy_cube", "resources/game/td_3105/Model/cube/cube.obj", mesh);
		material->materials = No::ModelLoader::GetMaterial("commentbout_rail_enemy_cube");
		material->color = { 1.0f, 1.0f, 1.0f, 1.0f };
		material->psoName = L"Renderer : Default PSO";
		material->psoId = NoEngine::Render::GetPSOID(material->psoName);
		material->rootSigId = NoEngine::Render::GetRootSignatureID(material->psoName);

		auto* enemy = registry.AddComponent<EnemyComponent>(enemyEntity);
		enemy->maxHp = std::max(1, params.hp);
		enemy->hp = enemy->maxHp;
		enemy->moveSpeed = std::max(0.0f, params.moveSpeed);
		enemy->moveDirection = direction;
		enemy->groupId = params.spawnGroupId;

		auto* commonHealth = registry.AddComponent<HealthComponent>(enemyEntity);
		commonHealth->hp = enemy->hp;
		commonHealth->maxHp = enemy->maxHp;
		commonHealth->isDead = false;
		commonHealth->lastDamageTaken = 0;

		auto* collider3D = registry.AddComponent<CommentBoutCollision::Collider3DComponent>(enemyEntity);
		collider3D->shapeType = CommentBoutCollision::ShapeType3D::Box;
		collider3D->useScaleAsBox = true;
		collider3D->boxSizeMultiplier = { 1.0f, 1.0f, 1.0f };
		collider3D->collisionLayer = CommentBout::CollisionLayer::CBEnemy;
		collider3D->collisionMask = CommentBout::CollisionLayer::CBPlayerAttack;

		auto* projected = registry.AddComponent<CommentBoutCollision::ProjectedColliderComponent>(enemyEntity);
		projected->source3DEntity = enemyEntity;
	}
}
}

void SpawnRailEnemies(No::Registry& registry, const RailEnemySpawnEventParams& params) {
	SpawnRailEnemiesInternal(registry, params);
}

void EnemySystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(registry);
	static_cast<void>(deltaTime);
}

