#include "stdafx.h"
#include "StageIntroDisplaySystem.h"
#include "../../Component/Stage/StageIntroDisplayComponent.h"

void StageIntroDisplaySystem::Update(No::Registry& registry, float deltaTime) {
	for (auto entity : registry.View<No::TextComponent, StageIntroDisplayComponent>()) {
		auto* text = registry.GetComponent<No::TextComponent>(entity);
		auto* display = registry.GetComponent<StageIntroDisplayComponent>(entity);
		if (!text || !display || !text->isVisible) continue;

		display->elapsed += deltaTime;
		const float fadeInStart = display->startDelay;
		const float fadeInEnd = fadeInStart + std::max(display->fadeInDuration, 0.0f);
		const float fadeOutStart = fadeInEnd + std::max(display->holdDuration, 0.0f);
		const float fadeOutEnd = fadeOutStart + std::max(display->fadeOutDuration, 0.0f);

		float alpha = 0.0f;
		if (display->elapsed < fadeInStart) {
			alpha = 0.0f;
		} else if (display->elapsed < fadeInEnd && display->fadeInDuration > 0.0f) {
			alpha = (display->elapsed - fadeInStart) / display->fadeInDuration;
		} else if (display->elapsed < fadeOutStart) {
			alpha = 1.0f;
		} else if (display->elapsed < fadeOutEnd && display->fadeOutDuration > 0.0f) {
			alpha = 1.0f - (display->elapsed - fadeOutStart) / display->fadeOutDuration;
		} else if (display->elapsed < fadeOutEnd) {
			alpha = 0.0f;
		} else {
			alpha = 0.0f;
			text->isVisible = false;
		}
		text->color.a = std::clamp(alpha, 0.0f, 1.0f);
	}
}

REGISTER_SYSTEM(::StageIntroDisplaySystem, "StageIntroDisplaySystem", "UI")
