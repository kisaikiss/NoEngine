#include "stdafx.h"
#include "ExBackgroundSystem.h"
#include "../../Game/RabbitdokuTag.h"

void ExBackgroundSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<No::SpriteComponent, ExBackgroundTag>();
	for (auto e : view) {
		auto* s = registry.GetComponent<No::SpriteComponent>(e);
		s->uv.x += deltaTime / 10.f;
		s->uv.y += deltaTime / 10.f;
	}
}
