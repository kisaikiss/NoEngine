#include "stdafx.h"
#include "RabbitdokuLoadSystem.h"
#include "application/RabbitdokuOdyssey3Plus/Component/RabbitdokuComponent.h"

void RabbitdokuLoadSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	if (isStartFrame_) {
		auto view = registry.View<Rabbitdoku, No::Transform2DComponent>();
		for (auto e : view) {
			auto* t = registry.GetComponent<No::Transform2DComponent>(e);
			auto* r = registry.GetComponent<Rabbitdoku>(e);
			t->translate = r->respawnPoint;
		}


		isStartFrame_ = false;
	}
}
