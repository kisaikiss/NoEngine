#include "stdafx.h"
#include "SpringSystem.h"
#include "../../Game/RabbitdokuTag.h"


void SpringSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto view = registry.View<SpringComponent, No::Animator2DComponent>();
	for (auto e : view) {
		auto* a = registry.GetComponent<No::Animator2DComponent>(e);
		if (a->isAnimationEnd) {
			a->framesNum = 1;
		}
	}
}
