#include "stdafx.h"
#include "HpBarViewSystem.h"
#include "application/CommentBout/Component/HpBarComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/GameTag.h"
#include "externals/nlohmann/json.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <filesystem>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

namespace {
const char* kHpBarConfigPath = "resources/game/td_3105/Data/StageData/RailData/Stage_01_hpbar.json";

No::Entity ResolveTargetEntity(No::Registry& registry, No::Entity barEntity) {
	if (registry.Has<CBBossHpBarTag>(barEntity)) {
		auto view = registry.View<CBBossTag, HealthComponent>();
		auto it = view.begin();
		return (it == view.end()) ? No::nullEntity : *it;
	}
	if (registry.Has<CBPlayerHpBarTag>(barEntity)) {
		auto view = registry.View<CBPlayerTag, HealthComponent>();
		auto it = view.begin();
		return (it == view.end()) ? No::nullEntity : *it;
	}
	return No::nullEntity;
}

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

void SyncBarVisualProps(No::Registry& registry, const HpBarComponent& bar) {
	auto apply = [&](No::Entity e, const No::Color& color, int orderOffset) {
		auto* s = registry.GetComponent<No::SpriteComponent>(e);
		if (!s) {
			return;
		}
		s->layer = bar.layer;
		s->orderInLayer = bar.orderBase + orderOffset;
		s->color = color;
	};
	apply(bar.backEntity, bar.backColor, 0);
	apply(bar.delayedEntity, bar.delayedColor, 1);
	apply(bar.fillEntity, bar.fillColor, 2);
}

void UpdateBarTransform(No::Registry& registry, No::Entity entity, const No::Vector2& anchorLeft, const No::Vector2& size) {
	auto* t = registry.GetComponent<No::Transform2DComponent>(entity);
	if (!t) {
		return;
	}
	t->translate = anchorLeft;
	t->scale = size;
}

nlohmann::json ColorToJson(const No::Color& c) {
	return { c.r, c.g, c.b, c.a };
}

void JsonToColor(const nlohmann::json& n, No::Color& out) {
	if (!n.is_array() || n.size() < 4) {
		return;
	}
	out = No::Color(n[0].get<float>(), n[1].get<float>(), n[2].get<float>(), n[3].get<float>());
}

void SaveBarNode(nlohmann::json& node, const HpBarComponent& b) {
	node["anchor"] = { b.anchor.x, b.anchor.y };
	node["size"] = { b.size.x, b.size.y };
	node["layer"] = b.layer;
	node["orderBase"] = b.orderBase;
	node["delayedLerpSpeed"] = b.delayedLerpSpeed;
	node["backColor"] = ColorToJson(b.backColor);
	node["delayedColor"] = ColorToJson(b.delayedColor);
	node["fillColor"] = ColorToJson(b.fillColor);
}

void LoadBarNode(const nlohmann::json& node, HpBarComponent& b) {
	if (node.contains("anchor") && node["anchor"].is_array() && node["anchor"].size() >= 2) {
		b.anchor = { node["anchor"][0].get<float>(), node["anchor"][1].get<float>() };
	}
	if (node.contains("size") && node["size"].is_array() && node["size"].size() >= 2) {
		b.size = { node["size"][0].get<float>(), node["size"][1].get<float>() };
	}
	if (node.contains("layer") && node["layer"].is_number_integer()) {
		b.layer = node["layer"].get<int>();
	}
	if (node.contains("orderBase") && node["orderBase"].is_number_integer()) {
		b.orderBase = node["orderBase"].get<int>();
	}
	if (node.contains("delayedLerpSpeed") && node["delayedLerpSpeed"].is_number()) {
		b.delayedLerpSpeed = node["delayedLerpSpeed"].get<float>();
	}
	if (node.contains("backColor")) {
		JsonToColor(node["backColor"], b.backColor);
	}
	if (node.contains("delayedColor")) {
		JsonToColor(node["delayedColor"], b.delayedColor);
	}
	if (node.contains("fillColor")) {
		JsonToColor(node["fillColor"], b.fillColor);
	}
}

void SaveBars(No::Registry& registry) {
	nlohmann::json j;
	for (auto e : registry.View<CBBossHpBarTag, HpBarComponent>()) {
		auto* b = registry.GetComponent<HpBarComponent>(e);
		if (!b) continue;
		SaveBarNode(j["boss"], *b);
		break;
	}
	for (auto e : registry.View<CBPlayerHpBarTag, HpBarComponent>()) {
		auto* b = registry.GetComponent<HpBarComponent>(e);
		if (!b) continue;
		SaveBarNode(j["player"], *b);
		break;
	}

	std::filesystem::create_directories(std::filesystem::path(kHpBarConfigPath).parent_path());
	std::ofstream ofs(kHpBarConfigPath);
	if (ofs) {
		ofs << j.dump(2);
	}
}

void LoadBars(No::Registry& registry) {
	std::ifstream ifs(kHpBarConfigPath);
	if (!ifs) {
		return;
	}
	nlohmann::json j;
	ifs >> j;

	if (j.contains("boss")) {
		for (auto e : registry.View<CBBossHpBarTag, HpBarComponent>()) {
			auto* b = registry.GetComponent<HpBarComponent>(e);
			if (!b) continue;
			LoadBarNode(j["boss"], *b);
			break;
		}
	}
	if (j.contains("player")) {
		for (auto e : registry.View<CBPlayerHpBarTag, HpBarComponent>()) {
			auto* b = registry.GetComponent<HpBarComponent>(e);
			if (!b) continue;
			LoadBarNode(j["player"], *b);
			break;
		}
	}
}

#ifdef USE_IMGUI
void DrawBarEditor(const char* label, HpBarComponent& bar) {
	if (!ImGui::TreeNode(label)) {
		return;
	}
	ImGui::DragFloat2("Anchor", &bar.anchor.x, 1.0f);
	ImGui::DragFloat2("Size", &bar.size.x, 1.0f, 1.0f, 4000.0f);
	ImGui::DragInt("Layer", &bar.layer, 1.0f, 0, 200);
	ImGui::DragInt("Order Base", &bar.orderBase, 1.0f, -2000, 2000);
	ImGui::DragFloat("Delayed Lerp Speed", &bar.delayedLerpSpeed, 0.01f, 0.01f, 20.0f);
	ImGui::ColorEdit4("Back Color", &bar.backColor.r);
	ImGui::ColorEdit4("Delayed Color", &bar.delayedColor.r);
	ImGui::ColorEdit4("Fill Color", &bar.fillColor.r);
	ImGui::TreePop();
}
#endif
}

void HpBarViewSystem::Update(No::Registry& registry, float deltaTime)
{
	GameResourceComponent* resource = nullptr;
	for (auto e : registry.View<CBGameResourceTag, GameResourceComponent>()) {
		resource = registry.GetComponent<GameResourceComponent>(e);
		if (resource) {
			break;
		}
	}
	if (!resource) {
		return;
	}

	static const No::Registry* loadedRegistry = nullptr;
	if (loadedRegistry != &registry) {
		LoadBars(registry);
		loadedRegistry = &registry;
	}

#ifdef USE_IMGUI
	ImGui::Begin("HP Bar Editor");
	if (ImGui::Button("Save HPBar")) {
		SaveBars(registry);
	}
	ImGui::SameLine();
	if (ImGui::Button("Load HPBar")) {
		LoadBars(registry);
	}
	{
		auto bossView = registry.View<CBBossHpBarTag, HpBarComponent>();
		auto it = bossView.begin();
		if (it != bossView.end()) {
			auto* bar = registry.GetComponent<HpBarComponent>(*it);
			if (bar) {
				DrawBarEditor("Boss Bar", *bar);
			}
		}
	}
	{
		auto playerView = registry.View<CBPlayerHpBarTag, HpBarComponent>();
		auto it = playerView.begin();
		if (it != playerView.end()) {
			auto* bar = registry.GetComponent<HpBarComponent>(*it);
			if (bar) {
				DrawBarEditor("Player Bar", *bar);
			}
		}
	}
	ImGui::End();
#endif

	for (auto e : registry.View<HpBarComponent>()) {
		auto* bar = registry.GetComponent<HpBarComponent>(e);
		if (!bar) {
			continue;
		}

		if (bar->targetEntity == No::nullEntity || !registry.Has<HealthComponent>(bar->targetEntity)) {
			bar->targetEntity = ResolveTargetEntity(registry, e);
		}

		EnsureBarSprites(registry, *bar, GetGameTextureOrWhite(*resource, CommentBoutResourceKey::kWhiteTexture));
		SyncBarVisualProps(registry, *bar);

		const bool hasTarget = (bar->targetEntity != No::nullEntity) && registry.Has<HealthComponent>(bar->targetEntity);
		float ratio = 0.0f;
		if (hasTarget) {
			auto* health = registry.GetComponent<HealthComponent>(bar->targetEntity);
			if (health && health->maxHp > 0) {
				ratio = std::max(0.0f, std::min(1.0f, static_cast<float>(health->hp) / static_cast<float>(health->maxHp)));
				if (bar->prevHp < 0.0f) {
					bar->displayRatio = ratio;
					bar->delayedRatio = ratio;
				} else if (static_cast<float>(health->hp) < bar->prevHp) {
					bar->shakeTime = 0.20f;
				}
				bar->prevHp = static_cast<float>(health->hp);
			}
		} else {
			if (bar->prevHp < 0.0f) {
				bar->displayRatio = 0.0f;
				bar->delayedRatio = 0.0f;
			}
			ratio = 0.0f;
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

		const No::Vector2 leftAnchor = { bar->anchor.x - bar->size.x * 0.5f + shakeX, bar->anchor.y };
		UpdateBarTransform(registry, bar->backEntity, leftAnchor, bar->size);
		UpdateBarTransform(registry, bar->delayedEntity, leftAnchor, { bar->size.x * bar->delayedRatio, bar->size.y });
		UpdateBarTransform(registry, bar->fillEntity, leftAnchor, { bar->size.x * bar->displayRatio, bar->size.y });
	}
}
