#include "stdafx.h"
#include "GrassReactionSystem.h"
#include "application/CommentBout/FieldObject/Component/GrassReactionComponent.h"
#include "application/CommentBout/FieldObject/Component/HitBalloonComponent.h"
#include "application/CommentBout/Component/LifetimeComponent.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Collision/Component/Collider3DComponent.h"
#include "application/CommentBout/Collision/Component/ProjectedColliderComponent.h"

void GrassReactionSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);

	GameResourceComponent* gameResource = nullptr;
	auto resourceView = registry.View<CBGameResourceTag, GameResourceComponent>();
	for (auto entity : resourceView) {
		gameResource = registry.GetComponent<GameResourceComponent>(entity);
		if (gameResource) break;
	}

	auto grassView = registry.View<CBGrassTag, GrassReactionComponent, CommentBoutCollision::Collider3DComponent, CommentBoutCollision::ProjectedColliderComponent>();
	for (auto entity : grassView) {
		auto* reaction = registry.GetComponent<GrassReactionComponent>(entity);
		auto* projected = registry.GetComponent<CommentBoutCollision::ProjectedColliderComponent>(entity);
		if (!reaction || !projected) continue;

		const bool hitNow = projected->isColliding;
		const bool hitPrev = reaction->wasColliding;

		if (hitNow && !hitPrev && projected->isVisible && gameResource) {
			No::Vector2 anchor{ projected->screenMax.x, projected->screenMin.y };

			auto effectEntity = registry.GenerateEntity();

			auto* transform2D = registry.AddComponent<No::Transform2DComponent>(effectEntity);
			transform2D->translate = anchor + reaction->effectOffset;
			transform2D->scale = reaction->effectSize;

			auto* sprite = registry.AddComponent<No::SpriteComponent>(effectEntity);
			sprite->layer = reaction->effectLayer;
			sprite->textureHandle = gameResource->whiteTexture;
			sprite->color = { 0.0f, 1.0f, 0.0f, 1.0f };

			auto* lifetime = registry.AddComponent<LifetimeComponent>(effectEntity);
			lifetime->remainingTime = reaction->effectLifetime;

			auto* balloon = registry.AddComponent<HitBalloonComponent>(effectEntity);
			balloon->sourceEntity = entity;
			balloon->localOffset = reaction->effectOffset;
			balloon->sizeRatio = reaction->sizeRatio;
			balloon->anchorType = HitBalloonComponent::AnchorType::TopRight;

			registry.AddComponent<CBGrassHitEffectTag>(effectEntity);
		}

		reaction->wasColliding = hitNow;
	}
}
