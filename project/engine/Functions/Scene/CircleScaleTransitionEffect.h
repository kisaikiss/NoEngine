#pragma once
#include "engine/Functions/Scene/ITransitionEffect.h"
#include "engine/Math/MathInclude.h"

namespace NoEngine {
namespace Scene {
class CircleScaleTransitionEffect : public ITransitionEffect {
public:
	void OnFadingOutStart(ECS::Registry& registry) override { CreateOverlay(registry, 0.f, 0.f); }
	void UpdateFadingOut(ECS::Registry& registry, float t) override {
		float eased = Easing::ApplyEasing(Easing::EasingType::EaseInOutSine, t);
		UpdateOverlay(registry, eased, eased);
	}
	void OnFadingInStart(ECS::Registry& registry) override {
		overlayEntity_ = ECS::INVALID_ENTITY;
		CreateOverlay(registry, 1.f, 1.f);
	}
	void UpdateFadingIn(ECS::Registry& registry, float t) override {
		float eased = Easing::ApplyEasing(Easing::EasingType::EaseInOutSine, t);
		float s = 1.f - eased;
		UpdateOverlay(registry, s, s);
	}
	void OnFinished(ECS::Registry& registry) override {
		if (overlayEntity_ != ECS::INVALID_ENTITY) registry.DestroyEntity(overlayEntity_);
		overlayEntity_ = ECS::INVALID_ENTITY;
	}

private:
	ECS::Entity overlayEntity_ = ECS::INVALID_ENTITY;
	void CreateOverlay(ECS::Registry& registry, float initialAlpha, float initialScale);
	void UpdateOverlay(ECS::Registry& registry, float alpha, float scale);
};
}
}