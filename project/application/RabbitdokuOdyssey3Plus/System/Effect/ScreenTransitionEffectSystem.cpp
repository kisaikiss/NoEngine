#include "stdafx.h"
#include "ScreenTransitionEffectSystem.h"
#include "../../Game/RabbitdokuTag.h"

void ScreenTransitionEffectSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<TransitionInTag, No::SpriteComponent, No::Transform2DComponent>();
	for (auto e : view) {
		(void)e;
	}
	(void)deltaTime;
}
