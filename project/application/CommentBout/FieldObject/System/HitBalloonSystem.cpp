#include "stdafx.h"
#include "HitBalloonSystem.h"
#include "application/CommentBout/FieldObject/Component/HitBalloonComponent.h"
#include "application/CommentBout/Collision/Component/ProjectedColliderComponent.h"

void HitBalloonSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);

	auto view = registry.View<HitBalloonComponent, No::Transform2DComponent>();
	for (auto entity : view) {
		auto* balloon = registry.GetComponent<HitBalloonComponent>(entity);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(entity);
		if (!balloon || !transform) continue;

		auto* projected = registry.GetComponent<CommentBoutCollision::ProjectedColliderComponent>(balloon->sourceEntity);
		if (!projected || !projected->isVisible) {
			transform->translate = { -9999.f, -9999.f };
			continue;
		}

		No::Vector2 anchor{};
		switch (balloon->anchorType) {
		case HitBalloonComponent::AnchorType::TopRight:
			anchor = { projected->screenMax.x, projected->screenMin.y };
			break;
		case HitBalloonComponent::AnchorType::Top:
			anchor = { projected->screenPosition.x, projected->screenMin.y };
			break;
		case HitBalloonComponent::AnchorType::TopLeft:
			anchor = { projected->screenMin.x, projected->screenMin.y };
			break;
		}
		transform->translate = anchor + balloon->localOffset;

		const bool doScaleX = balloon->sizeRatio.x > 0.f;
		const bool doScaleY = balloon->sizeRatio.y > 0.f;
		if (doScaleX || doScaleY) {
			const float grassScreenW = projected->screenMax.x - projected->screenMin.x;
			const float grassScreenH = projected->screenMax.y - projected->screenMin.y;

			if (doScaleX && grassScreenW > 0.f) {
				transform->scale.x = grassScreenW * balloon->sizeRatio.x;
			}
			if (doScaleY && grassScreenH > 0.f) {
				transform->scale.y = grassScreenH * balloon->sizeRatio.y;
			}
		}
	}
}
