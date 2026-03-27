#include "stdafx.h"
#include "DamageFlashSystem.h"
#include "application/CommentBout/Component/DamageFlashComponent.h"
#include <algorithm>
#include <cmath>

void DamageFlashSystem::Update(No::Registry& registry, float deltaTime)
{
	auto view = registry.View<DamageFlashComponent>();
	for (auto entity : view) {
		auto* flash = registry.GetComponent<DamageFlashComponent>(entity);
		if (!flash) {
			continue;
		}

		if (flash->timer > 0.0f) {
			flash->timer -= deltaTime;
			if (flash->timer < 0.0f) {
				flash->timer = 0.0f;
			}
		}

		const bool active = (flash->timer > 0.0f);
		No::Color outColor = active ? flash->flashColor : flash->baseColor;

		if (active && flash->blinkEnabled) {
			const float safeDuration = std::max(0.0001f, flash->duration);
			const float elapsed = safeDuration - std::min(safeDuration, flash->timer);
			const float cycle = std::sin(elapsed * std::max(0.0f, flash->blinkHz) * 2.0f * 3.14159265f);
			const float t = 0.5f + 0.5f * cycle;
			const float minA = std::max(0.0f, std::min(1.0f, flash->minAlpha));
			const float maxA = std::max(minA, std::min(1.0f, flash->maxAlpha));
			outColor.a = minA + (maxA - minA) * t;
		}

		if (flash->affectSprite) {
			auto* sprite = registry.GetComponent<No::SpriteComponent>(entity);
			if (sprite) {
				sprite->color = outColor;
			}
		}
		if (flash->affectMaterial) {
			auto* material = registry.GetComponent<No::MaterialComponent>(entity);
			if (material) {
				material->color = outColor;
			}
		}
	}
}
