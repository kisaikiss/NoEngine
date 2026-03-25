#include "stdafx.h"
#include "BossHpBarViewSystem.h"
#include "application/CommentBout/Component/BossHpBarComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/GameTag.h"
#include <algorithm>
#include <cmath>

namespace {
No::Entity FindBossEntity(No::Registry& registry) {
	auto view = registry.View<CBBossTag, HealthComponent>();
	auto it = view.begin();
	if (it == view.end()) {
		return No::nullEntity;
	}
	return *it;
}

void EnsureBarSprites(No::Registry& registry, BossHpBarComponent& bar, NoEngine::TextureRef whiteTexture) {
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
		s->layer = 90;
		s->orderInLayer = order;
		s->pivot = { 0.0f, 0.5f };
		s->color = color;
		return e;
	};

	bar.backEntity = makeBar(No::Color(0.1f, 0.1f, 0.1f, 0.75f), 0);
	bar.delayedEntity = makeBar(No::Color(0.95f, 0.2f, 0.2f, 0.9f), 1);
	bar.fillEntity = makeBar(No::Color(0.2f, 0.9f, 0.25f, 0.95f), 2);
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

void BossHpBarViewSystem::Update(No::Registry& registry, float deltaTime)
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

	auto barView = registry.View<CBBossHpBarTag, BossHpBarComponent>();
	for (auto barEntity : barView) {
		auto* bar = registry.GetComponent<BossHpBarComponent>(barEntity);
		if (!bar) {
			continue;
		}

		if (bar->bossEntity == No::nullEntity || !registry.Has<HealthComponent>(bar->bossEntity)) {
			bar->bossEntity = FindBossEntity(registry);
		}
		if (bar->bossEntity == No::nullEntity) {
			continue;
		}

		EnsureBarSprites(registry, *bar, resource->whiteTexture);
		auto* health = registry.GetComponent<HealthComponent>(bar->bossEntity);
		if (!health || health->maxHp <= 0) {
			continue;
		}

		const float ratio = std::max(0.0f, std::min(1.0f, static_cast<float>(health->hp) / static_cast<float>(health->maxHp)));
		bar->displayRatio = ratio;
		bar->delayedRatio = std::max(ratio, bar->delayedRatio - bar->delayedLerpSpeed * deltaTime);

		if (bar->prevHp >= 0.0f && static_cast<float>(health->hp) < bar->prevHp) {
			bar->shakeTime = 0.20f;
		}
		bar->prevHp = static_cast<float>(health->hp);

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
