#include "stdafx.h"
#include "OpeningPanelSystem.h"
#include "application/ClockworksDisease/Component/Opening/OpeningPanelComponent.h"

void OpeningPanelSystem::Update(No::Registry& registry, float deltaTime) {
	for (auto e : registry.View<OpeningPanelComponent, No::Transform2DComponent, No::SpriteComponent>()) {
		auto* panel = registry.GetComponent<OpeningPanelComponent>(e);
		panel->appearanceTimer += deltaTime;
		if (panel->appearanceTimer > panel->timeUntilAppearance) {
			registry.GetComponent<No::SpriteComponent>(e)->isVisible = true;
			auto* transform = registry.GetComponent<No::Transform2DComponent>(e);

			panel->appearanceEffectTimer += deltaTime;
			transform->scale = No::Lerp(No::Vector2::ZERO, panel->panelScale, No::ApplyEasing(No::EasingType::EaseInOutExpo, panel->appearanceEffectTimer));
			// 演出が終了すれば独自の動きは終了なので、コンポーネントを外す
			if (panel->appearanceEffectTimer > panel->appearanceEffectFinishTime) {
				registry.RemoveComponent<OpeningPanelComponent>(e);
			}

		} else {
			registry.GetComponent<No::SpriteComponent>(e)->isVisible = false;
			panel->panelScale = registry.GetComponent<No::Transform2DComponent>(e)->scale;
		}
	}
}
