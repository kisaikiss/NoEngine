#include "stdafx.h"
#include "GameProgressInitSystem.h"
#include "../../Component/Game/GameProgressComponent.h"

void GameProgressInitSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	for (auto e : registry.View<GameProgressComponent>()) {
		auto* progress = registry.GetComponent<GameProgressComponent>(e);
		if (progress->totalCounted) continue;

		uint32_t count = 0;
		for (auto item : registry.View<CollectibleItemTag>()) {
			static_cast<void>(item);
			count++;
		}

		// まだ1つも読み込まれていない場合はシーンJSONの読み込みが済んでいない可能性があるので
		// 次フレームに再試行する
		if (count == 0) continue;

		progress->totalItemCount = count;
		progress->totalCounted = true;
	}
}