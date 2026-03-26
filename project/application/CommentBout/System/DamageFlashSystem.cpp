#include "stdafx.h"
#include "DamageFlashSystem.h"
#include "application/CommentBout/Component/DamageFlashComponent.h"

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
		const No::Color outColor = active ? flash->flashColor : flash->baseColor;

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
