#include "stdafx.h"
#include "ItemGetSystem.h"
#include "../Game/CollisionEvents.h"
#include "../../Component/Item/ItemComponent.h"
#include "../../Component/Player/PlayerComponent.h"
#include "../../Component/Game/GameProgressComponent.h"
#include "GameResult.h"

namespace {

void IncrementCollectedCount(No::Registry& registry, No::Entity item) {
	if (!registry.Has<CollectibleItemTag>(item)) return;
	for (auto e : registry.View<GameProgressComponent>()) {
		registry.GetComponent<GameProgressComponent>(e)->collectedItemCount++;
	}
}

void TriggerGameClear(No::Registry& registry) {
	GameResult::Data result;
	for (auto e : registry.View<GameProgressComponent>()) {
		auto* progress = registry.GetComponent<GameProgressComponent>(e);
		result.totalItemCount = progress->totalItemCount;
		result.collectedItemCount = progress->collectedItemCount;
		result.clearTime = progress->elapsedTime;
	}
	GameResult::Set(result);

	No::SceneChangeEvent sceneChangeEvent;
	sceneChangeEvent.nextScene = "GameClearScene";
	registry.EmitEvent(sceneChangeEvent);
}

} // namespace

void ItemGetSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	auto events = registry.PollAllEvents<ItemGetEvent>();
	for (auto event : events) {
		// ゴールアイテムなら他の処理より先にクリア判定する
		if (registry.Has<GoalItemTag>(event.item)) {
			TriggerGameClear(registry);
			registry.DestroyEntity(event.item);
			continue;
		}

		if (registry.Has<PowerItemComponent>(event.item)) {
			IncrementCollectedCount(registry, event.item);
			registry.AddComponent<No::EffectEmitTag>(event.item);
			registry.DestroyEntity(event.item);
			constexpr uint32_t kBadgePower = 1;
			registry.GetComponent<LevelComponent>(event.player)->power += kBadgePower;
			continue;
		}

		if (registry.Has<BigPowerItemComponent>(event.item)) {
			IncrementCollectedCount(registry, event.item);
			registry.AddComponent<BigPowerGetTag>(event.item);
			registry.GetComponent<No::ParticleEmitterComponent>(event.item)->active = true;
			registry.RemoveComponent<No::SphereCollider>(event.item);

			registry.GetComponent<LevelComponent>(event.player)->power += registry.GetComponent<BigPowerItemComponent>(event.item)->grantPower;
		}
	}
}