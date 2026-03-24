#include "stdafx.h"
#include "EnemySpawnSystem.h"
#include "application/CommentBout/Component/SpawnEnemyRequestComponent.h"
#include "application/CommentBout/System/EnemySystem.h"

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
