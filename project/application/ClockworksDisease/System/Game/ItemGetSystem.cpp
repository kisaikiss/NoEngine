#include "stdafx.h"
#include "ItemGetSystem.h"
#include "../Game/CollisionEvents.h"
#include "../../Component/Item/ItemComponent.h"
#include "../../Component/Player/PlayerComponent.h"

void ItemGetSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	auto events = registry.PollAllEvents<ItemGetEvent>();
	for (auto event : events) {
		if (registry.Has<PowerItemComponent>(event.item)) {
			registry.AddComponent<No::EffectEmitTag>(event.item);
			registry.DestroyEntity(event.item);
			constexpr uint32_t kBadgePower = 1;
			registry.GetComponent<LevelComponent>(event.player)->power += kBadgePower;
			continue;
		}

		if (registry.Has<BigPowerItemComponent>(event.item)) {
			registry.AddComponent<BigPowerGetTag>(event.item);
			registry.GetComponent<No::ParticleEmitterComponent>(event.item)->active = true;
			registry.RemoveComponent<No::SphereCollider>(event.item);

			constexpr uint32_t kBigBadgePower = 5;
			registry.GetComponent<LevelComponent>(event.player)->power += kBigBadgePower;
		}
	}
}
