#include "BackGroundEffectSystem.h"
#include "../Component/BackGroundComponent.h"

void BackGroundEffectSystem::Update(No::Registry& registry, float deltaTime)
{
	auto view = registry.View<BackGroundComponent>();

	for (auto entity : view)
	{
		auto* backGround = registry.GetComponent<BackGroundComponent>(entity);
		backGround->time += deltaTime;
	}
}
