#include "stdafx.h"
#include "BackgroundAttachSystem.h"
#include "../../Game/RabbitdokuTag.h"

void BackgroundAttachSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	No::Transform2DComponent* cameraT = nullptr;
	auto cameraView = registry.View<No::ActiveCamera2DTag, No::Transform2DComponent>();
	for (auto e : cameraView) {
		cameraT = registry.GetComponent<No::Transform2DComponent>(e);
	}
	auto view = registry.View<No::Transform2DComponent, No::SpriteComponent, MainBackgroundTag>();
	for (auto e : view) {
		auto* t = registry.GetComponent<No::Transform2DComponent>(e);
		if (!cameraT) {
			t->parent = nullptr;
		} else {
			t->parent = cameraT;
		}
	}

}
