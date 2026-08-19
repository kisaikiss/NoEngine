#pragma once
#include "engine/Functions/Scene/ITransitionEffect.h"
#include "engine/Math/MathInclude.h"

namespace NoEngine {
namespace Scene {
class FadeTransitionEffect : public ITransitionEffect {
public:
	explicit FadeTransitionEffect(Math::Color color = Math::Color::BLACK) : color_(color) {}
	void OnFadingOutStart(ECS::Registry& registry) override { CreateOverlay(registry, 0.f); }
	void UpdateFadingOut(ECS::Registry& registry, float t) override {
		UpdateOverlay(registry, Easing::ApplyEasing(Easing::EasingType::EaseInOutSine, t));
	}
	void OnFadingInStart(ECS::Registry& registry) override {
		overlayEntity_ = ECS::INVALID_ENTITY;
		CreateOverlay(registry, 1.f);
	}
	void UpdateFadingIn(ECS::Registry& registry, float t) override {
		float eased = Easing::ApplyEasing(Easing::EasingType::EaseInOutSine, t);
		UpdateOverlay(registry, 1.f - eased);
	}
	void OnFinished(ECS::Registry& registry) override {
		if (overlayEntity_ != ECS::INVALID_ENTITY) registry.DestroyEntity(overlayEntity_);
		overlayEntity_ = ECS::INVALID_ENTITY;
	}
private:
	Math::Color color_;
	ECS::Entity overlayEntity_ = ECS::INVALID_ENTITY;
	void CreateOverlay(ECS::Registry& registry, float initialAlpha);
	void UpdateOverlay(ECS::Registry& registry, float alpha);
};
}
}