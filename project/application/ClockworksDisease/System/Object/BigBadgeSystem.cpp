#include "stdafx.h"
#include "BigBadgeSystem.h"
#include "application/ClockworksDisease/Component/Item/ItemComponent.h"

void BigBadgeSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	for (auto e : registry.View<BigBadgeComponent, BigBadgeGetTag, No::TransformComponent>()) {
		auto* badge = registry.GetComponent<BigBadgeComponent>(e);
		if (!badge->playerTransform)continue;
		auto* transform = registry.GetComponent<No::TransformComponent>(e);
		transform->translate = badge->playerTransform->GetWorldPosition();
	}
}
