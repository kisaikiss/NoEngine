#include "stdafx.h"
#include "ItemGetSystem.h"
#include "../Game/CollisionEvents.h"
#include "../../Component/Item/ItemComponent.h"

void ItemGetSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	auto events = registry.PollAllEvents<ItemGetEvent>();
	for (auto event : events) {
		if (registry.Has<BadgeComponent>(event.item)) {
			registry.AddComponent<No::EffectEmitTag>(event.item);
			registry.DestroyEntity(event.item);
			continue;
		}

		if (registry.Has<BigBadgeComponent>(event.item)) {
			auto* badge = registry.GetComponent<BigBadgeComponent>(event.item);
			registry.AddComponent<BigBadgeGetTag>(event.item);
			badge->playerTransform = registry.GetComponent<No::TransformComponent>(event.player);
		}
	}
}
