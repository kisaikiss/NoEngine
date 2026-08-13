#include "stdafx.h"
#include "PlatformRideSystem.h"
#include "../../Component/Player/PlayerComponent.h"

void PlatformRideSystem::Update(No::Registry& registry, float deltaTime) {
    static_cast<void>(deltaTime);
    for (auto e : registry.View<PlayerComponent, No::TransformComponent, No::GroundStateComponent>()) {
        auto* ground = registry.GetComponent<No::GroundStateComponent>(e);
        if (!ground->isGrounded || ground->groundEntity == No::INVALID_ENTITY) continue;

        if (auto* delta = registry.GetComponent<No::PlatformDeltaComponent>(ground->groundEntity)) {
            registry.GetComponent<No::TransformComponent>(e)->translate += delta->frameDelta;
        }
    }
}