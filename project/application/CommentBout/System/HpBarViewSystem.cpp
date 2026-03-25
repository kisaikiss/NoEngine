#include "stdafx.h"
#include "HpBarViewSystem.h"
#include "application/CommentBout/Component/HpBarComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/GameTag.h"
#include <algorithm>
#include <cmath>

namespace {
void EnsureBarSprites(No::Registry& registry, HpBarComponent& bar, NoEngine::TextureRef whiteTexture) {
	if (bar.backEntity != No::nullEntity && bar.delayedEntity != No::nullEntity && bar.fillEntity != No::nullEntity) {
		return;
	}

	auto makeBar = [&](const No::Color& color, int order) -> No::Entity {
		auto e = registry.GenerateEntity();
		auto* t = registry.AddComponent<No::Transform2DComponent>(e);
		t->translate = bar.anchor;
		t->scale = bar.size;
		auto* s = registry.AddComponent<No::SpriteComponent>(e);
		s->textureHandle = whiteTexture;
		s->layer = bar.layer;
		s->orderInLayer = bar.orderBase + order;
		s->pivot = { 0.0f, 0.5f };
		s->color = color;
		return e;
	};

	bar.backEntity = makeBar(bar.backColor, 0);
	bar.delayedEntity = makeBar(bar.delayedColor, 1);
	bar.fillEntity = makeBar(bar.fillColor, 2);
}

void UpdateBarTransform(No::Registry& registry, No::Entity entity, const No::Vector2& anchorLeft, const No::Vector2& size) {
	auto* t = registry.GetComponent<No::Transform2DComponent>(entity);
	if (!t) {
		return;
	}
	t->translate = anchorLeft;
	t->scale = size;
}
}

void HpBarViewSystem::Update(No::Registry& registry, float deltaTime)
{
	GameResourceComponent* resource = nullptr;
	auto resView = registry.View<CBGameResourceTag, GameResourceComponent>();
	for (auto e : resView) {
		resource = registry.GetComponent<GameResourceComponent>(e);
		if (resource) {
			break;
		}
	}
	if (!resource) {
		return;
	}

	auto view = registry.View<HpBarComponent>();
	for (auto e : view) {
		auto* bar = registry.GetComponent<HpBarComponent>(e);
		if (!bar) {
			continue;
		}

		if (bar->targetEntity == No::nullEntity) {
			if (registry.Has<CBBossHpBarTag>(e)) {
				auto bossView = registry.View<CBBossTag, HealthComponent>();
				auto it = bossView.begin();
				if (it != bossView.end()) {
					bar->targetEntity = *it;
				}
			} else if (registry.Has<CBPlayerHpBarTag>(e)) {
				auto playerView = registry.View<CBPlayerTag, HealthComponent>();
				auto it = playerView.begin();
				if (it != playerView.end()) {
					bar->targetEntity = *it;
				}
			}
		}

		EnsureBarSprites(registry, *bar, resource->whiteTexture);

		float ratio = 0.0f;
		bool hasTarget = (bar->targetEntity != No::nullEntity) && registry.Has<HealthComponent>(bar->targetEntity);
		if (hasTarget) {
			auto* health = registry.GetComponent<HealthComponent>(bar->targetEntity);
			if (health && health->maxHp > 0) {
				ratio = std::max(0.0f, std::min(1.0f, static_cast<float>(health->hp) / static_cast<float>(health->maxHp)));
				if (bar->prevHp >= 0.0f && static_cast<float>(health->hp) < bar->prevHp) {
					bar->shakeTime = 0.20f;
				}
				bar->prevHp = static_cast<float>(health->hp);
			}
		}

		if (!hasTarget && !bar->followToZeroWhenDead) {
			continue;
		}

		bar->displayRatio = ratio;
		bar->delayedRatio = std::max(ratio, bar->delayedRatio - bar->delayedLerpSpeed * deltaTime);

		float shakeX = 0.0f;
		if (bar->shakeTime > 0.0f) {
			bar->shakeTime -= deltaTime;
			shakeX = std::sinf(bar->shakeTime * 90.0f) * 8.0f;
		}

		No::Vector2 leftAnchor = { bar->anchor.x - bar->size.x * 0.5f + shakeX, bar->anchor.y };
		UpdateBarTransform(registry, bar->backEntity, leftAnchor, bar->size);
		UpdateBarTransform(registry, bar->delayedEntity, leftAnchor, { bar->size.x * bar->delayedRatio, bar->size.y });
		UpdateBarTransform(registry, bar->fillEntity, leftAnchor, { bar->size.x * bar->displayRatio, bar->size.y });
	}
}
