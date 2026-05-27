#include "stdafx.h"
#include "SmokeEffectSystem.h"
#include "../../Game/RabbitdokuTag.h"

void SmokeEffectSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto view = registry.View<SmokeEffectTag, No::Animator2DComponent>();
	for (auto e : view) {
		if (registry.GetComponent<No::Animator2DComponent>(e)->isAnimationEnd) {
			registry.DestroyEntity(e);
		}
	}

}
