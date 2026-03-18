#include "stdafx.h"
#include "GrassReactionSystem.h"
#include "application/CommentBout/Component/GrassReactionComponent.h"
#include "application/CommentBout/Component/HitBalloonComponent.h"
#include "application/CommentBout/Component/LifetimeComponent.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/GameTag.h"
#include "application/TestApp/Component/Collider3DComponent.h"
#include "application/TestApp/Component/ProjectedColliderComponent.h"

void GrassReactionSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);

	GameResourceComponent* gameResource = nullptr;
	auto resourceView = registry.View<CBGameResourceTag, GameResourceComponent>();
	for (auto entity : resourceView) {
		gameResource = registry.GetComponent<GameResourceComponent>(entity);
		if (gameResource) break;
	}

	auto grassView = registry.View<CBGrassTag, GrassReactionComponent, TestApp::Collider3DComponent, TestApp::ProjectedColliderComponent>();
	for (auto entity : grassView) {
		auto* reaction = registry.GetComponent<GrassReactionComponent>(entity);
		auto* projected = registry.GetComponent<TestApp::ProjectedColliderComponent>(entity);
		if (!reaction || !projected) continue;

		const bool hitNow = projected->isColliding;
		const bool hitPrev = reaction->wasColliding;

		if (hitNow && !hitPrev && projected->isVisible && gameResource) {

			No::Vector2 anchor{ projected->screenMax.x, projected->screenMin.y };

			auto effectEntity = registry.GenerateEntity();

			auto* transform2D = registry.AddComponent<No::Transform2DComponent>(effectEntity);
			transform2D->translate = anchor + reaction->effectOffset;
			// scale 初期値は effectSize（固定ピクセル）
			// sizeRatio が {0,0} 以外なら HitBalloonSystem が草の投影サイズ比率で上書きする
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
			balloon->sizeRatio = reaction->sizeRatio;    // GrassReactionComponent と1対1で対応
			balloon->anchorType = HitBalloonComponent::AnchorType::TopRight;

			registry.AddComponent<CBGrassHitEffectTag>(effectEntity);
		}

		reaction->wasColliding = hitNow;
	}
}