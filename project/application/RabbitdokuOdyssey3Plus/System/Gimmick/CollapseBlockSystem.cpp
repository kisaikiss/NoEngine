#include "stdafx.h"
#include "CollapseBlockSystem.h"
#include "../../Game/RabbitdokuTag.h"
#include "application/RabbitdokuOdyssey3Plus/Game/RabbitdokuCollisionLayer.h"

void CollapseBlockSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<CollapseBlockComponent, No::SpriteComponent, No::CollisionBody, No::Animator2DComponent>();
	for (auto e : view) {
		auto* block = registry.GetComponent<CollapseBlockComponent>(e);
		if (block->startCollapse) {
			block->collapseTime -= deltaTime;
			if (block->collapseTime < 0.f) {
				auto* a = registry.GetComponent<No::Animator2DComponent>(e);
				a->currentAnimation = 2;
				if (!block->startBreak) {
					registry.GetComponent<No::SpriteComponent>(e)->uv.x = 0.f;
					block->startBreak = true;
				}
				auto* body = registry.GetComponent<No::CollisionBody>(e);
				body->type = No::BodyType::Through;
				registry.RemoveComponent<BlockTag>(e);
				registry.GetComponent<RabbitdokuCollisionLayerComponent>(e)->layer = RabbitdokuCollisionLayerComponent::None;
				if (a->isAnimationEnd) {
					registry.DestroyEntity(e);
				}
			} else {
				registry.GetComponent<No::SpriteComponent>(e)->pivot = No::GetRandomVal(No::Vector2(0.4f, 0.4f), No::Vector2(0.6f, 0.6f));
			}
		}
	}
}
