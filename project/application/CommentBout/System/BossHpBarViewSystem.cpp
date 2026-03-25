#include "stdafx.h"
#include "BossHpBarViewSystem.h"
#include "application/CommentBout/Component/BossHpBarComponent.h"
#include "application/CommentBout/Component/HpBarComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/GameTag.h"
#include "externals/nlohmann/json.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

namespace {
const char* kHpBarConfigPath = "resources/game/td_3105/RailData/Stage_01_hpbar.json";

No::Entity FindEntity(No::Registry& registry, bool isBoss) {
	if (isBoss) {
		auto v = registry.View<CBBossTag, HealthComponent>();
		auto it = v.begin();
		return (it == v.end()) ? No::nullEntity : *it;
	}
	auto v = registry.View<CBPlayerTag, HealthComponent>();
	auto it = v.begin();
	return (it == v.end()) ? No::nullEntity : *it;
}

void EnsureBarSprites(No::Registry& registry, HpBarComponent& bar, NoEngine::TextureRef whiteTexture) {
	if (bar.backEntity != No::nullEntity && bar.delayedEntity != No::nullEntity && bar.fillEntity != No::nullEntity) {
		return;
	}
	auto makeBar = [&](const No::Color& color, int order)->No::Entity {
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

void UpdateBarTransform(No::Registry& registry, No::Entity e, const No::Vector2& pos, const No::Vector2& size) {
	auto* t = registry.GetComponent<No::Transform2DComponent>(e);
	if (!t) return;
	t->translate = pos;
	t->scale = size;
}

void SaveBars(No::Registry& registry) {
	nlohmann::json j;
	auto bossView = registry.View<CBBossHpBarTag, HpBarComponent>();
	for (auto e : bossView) {
		auto* b = registry.GetComponent<HpBarComponent>(e);
		if (!b) continue;
		j["boss"]["anchor"] = { b->anchor.x, b->anchor.y };
		j["boss"]["size"] = { b->size.x, b->size.y };
	}
	auto playerView = registry.View<CBPlayerHpBarTag, HpBarComponent>();
	for (auto e : playerView) {
		auto* b = registry.GetComponent<HpBarComponent>(e);
		if (!b) continue;
		j["player"]["anchor"] = { b->anchor.x, b->anchor.y };
		j["player"]["size"] = { b->size.x, b->size.y };
	}
	std::ofstream ofs(kHpBarConfigPath);
	if (ofs) ofs << j.dump(2);
}

void LoadBars(No::Registry& registry) {
	std::ifstream ifs(kHpBarConfigPath);
	if (!ifs) return;
	nlohmann::json j;
	ifs >> j;

	if (j.contains("boss")) {
		for (auto e : registry.View<CBBossHpBarTag, HpBarComponent>()) {
			auto* b = registry.GetComponent<HpBarComponent>(e);
			if (!b) continue;
			auto& n = j["boss"];
			if (n.contains("anchor") && n["anchor"].is_array() && n["anchor"].size() >= 2) {
				b->anchor = { n["anchor"][0].get<float>(), n["anchor"][1].get<float>() };
			}
			if (n.contains("size") && n["size"].is_array() && n["size"].size() >= 2) {
				b->size = { n["size"][0].get<float>(), n["size"][1].get<float>() };
			}
		}
	}

	if (j.contains("player")) {
		for (auto e : registry.View<CBPlayerHpBarTag, HpBarComponent>()) {
			auto* b = registry.GetComponent<HpBarComponent>(e);
			if (!b) continue;
			auto& n = j["player"];
			if (n.contains("anchor") && n["anchor"].is_array() && n["anchor"].size() >= 2) {
				b->anchor = { n["anchor"][0].get<float>(), n["anchor"][1].get<float>() };
			}
			if (n.contains("size") && n["size"].is_array() && n["size"].size() >= 2) {
				b->size = { n["size"][0].get<float>(), n["size"][1].get<float>() };
			}
		}
	}
}
}

void BossHpBarViewSystem::Update(No::Registry& registry, float deltaTime)
{
	GameResourceComponent* resource = nullptr;
	auto resView = registry.View<CBGameResourceTag, GameResourceComponent>();
	for (auto e : resView) {
		resource = registry.GetComponent<GameResourceComponent>(e);
		if (resource) break;
	}
	if (!resource) return;

	if (registry.View<CBBossHpBarTag, HpBarComponent>().begin() == registry.View<CBBossHpBarTag, HpBarComponent>().end()) {
		auto e = registry.GenerateEntity();
		registry.AddComponent<CBBossHpBarTag>(e);
		auto* bar = registry.AddComponent<HpBarComponent>(e);
		bar->anchor = { 640.0f, 64.0f };
		bar->size = { 420.0f, 28.0f };
		bar->fillColor = No::Color(0.2f, 0.9f, 0.25f, 0.95f);
	}
	if (registry.View<CBPlayerHpBarTag, HpBarComponent>().begin() == registry.View<CBPlayerHpBarTag, HpBarComponent>().end()) {
		auto e = registry.GenerateEntity();
		registry.AddComponent<CBPlayerHpBarTag>(e);
		auto* bar = registry.AddComponent<HpBarComponent>(e);
		bar->anchor = { 160.0f, 660.0f };
		bar->size = { 280.0f, 22.0f };
		bar->fillColor = No::Color(0.2f, 0.7f, 1.0f, 0.95f);
	}

#ifdef USE_IMGUI
	ImGui::Begin("HP Bar Editor");
	if (ImGui::Button("Save HPBar")) SaveBars(registry);
	ImGui::SameLine();
	if (ImGui::Button("Load HPBar")) LoadBars(registry);
	ImGui::End();
#endif

	auto view = registry.View<HpBarComponent>();
	for (auto barEntity : view) {
		auto* bar = registry.GetComponent<HpBarComponent>(barEntity);
		if (!bar) continue;

		if (bar->targetEntity == No::nullEntity || !registry.Has<HealthComponent>(bar->targetEntity)) {
			if (registry.Has<CBBossHpBarTag>(barEntity)) {
				bar->targetEntity = FindEntity(registry, true);
			} else if (registry.Has<CBPlayerHpBarTag>(barEntity)) {
				bar->targetEntity = FindEntity(registry, false);
			}
		}

		EnsureBarSprites(registry, *bar, resource->whiteTexture);

		float ratio = 0.0f;
		if (bar->targetEntity != No::nullEntity && registry.Has<HealthComponent>(bar->targetEntity)) {
			auto* health = registry.GetComponent<HealthComponent>(bar->targetEntity);
			if (health && health->maxHp > 0) {
				ratio = std::max(0.0f, std::min(1.0f, static_cast<float>(health->hp) / static_cast<float>(health->maxHp)));
				if (bar->prevHp >= 0.0f && static_cast<float>(health->hp) < bar->prevHp) {
					bar->shakeTime = 0.2f;
				}
				bar->prevHp = static_cast<float>(health->hp);
			}
		}

		bar->displayRatio = ratio;
		bar->delayedRatio = std::max(ratio, bar->delayedRatio - bar->delayedLerpSpeed * deltaTime);
		if (ratio <= 0.0f && bar->delayedRatio < 0.001f) {
			bar->delayedRatio = 0.0f;
		}

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
