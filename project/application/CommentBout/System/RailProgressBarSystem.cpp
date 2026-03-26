#include "stdafx.h"
#include "RailProgressBarSystem.h"
#include "application/CommentBout/Component/RailProgressBarComponent.h"
#include "application/CommentBout/Component/RailCameraComponent.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/GameTag.h"
#include <algorithm>
#include <cmath>
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

namespace {
	float Clamp01(float v) {
		if (v < 0.0f) { return 0.0f; }
		if (v > 1.0f) { return 1.0f; }
		return v;
	}

	No::Entity CreateSpriteEntity(
		No::Registry& registry,
		const No::Vector2& position,
		const No::Vector2& scale,
		const No::Color& color,
		int layer,
		int order,
		const NoEngine::TextureRef& texture
	) {
		No::Entity e = registry.GenerateEntity();
		auto* t = registry.AddComponent<No::Transform2DComponent>(e);
		t->translate = position;
		t->scale = scale;
		auto* s = registry.AddComponent<No::SpriteComponent>(e);
		s->textureHandle = texture;
		s->layer = static_cast<uint32_t>(std::max(0, layer));
		s->orderInLayer = static_cast<uint32_t>(std::max(0, order));
		s->color = color;
		return e;
	}

	void EnsureSprites(
		No::Registry& registry,
		RailProgressBarComponent& bar,
		const NoEngine::TextureRef& startTex,
		const NoEngine::TextureRef& goalTex,
		const NoEngine::TextureRef& baseTex,
		const NoEngine::TextureRef& fillTex,
		const NoEngine::TextureRef& markerTex
	) {
		if (bar.startEntity != No::nullEntity && bar.goalEntity != No::nullEntity &&
			bar.barBaseEntity != No::nullEntity && bar.barFillEntity != No::nullEntity &&
			bar.playerMarkerEntity != No::nullEntity) {
			return;
		}

		const No::Vector2 barCenter = (bar.startPosition + bar.goalPosition) * 0.5f;
		const float barWidth = std::max(1.0f, std::abs(bar.goalPosition.x - bar.startPosition.x));
		const No::Vector2 baseSize = { barWidth, std::max(1.0f, bar.barHeight) };
		const No::Vector2 markerPos = { bar.startPosition.x, bar.startPosition.y };

		bar.startEntity = CreateSpriteEntity(registry, bar.startPosition, { 20.0f, 20.0f }, bar.startColor, bar.layer, bar.orderBase + 0, startTex);
		bar.goalEntity = CreateSpriteEntity(registry, bar.goalPosition, { 20.0f, 20.0f }, bar.goalColor, bar.layer, bar.orderBase + 1, goalTex);
		bar.barBaseEntity = CreateSpriteEntity(registry, barCenter, baseSize, bar.barBaseColor, bar.layer, bar.orderBase + 2, baseTex);
		bar.barFillEntity = CreateSpriteEntity(registry, barCenter, baseSize, bar.barFillColor, bar.layer, bar.orderBase + 3, fillTex);
		bar.playerMarkerEntity = CreateSpriteEntity(registry, markerPos, bar.markerSize, bar.markerColor, bar.layer, bar.orderBase + 4, markerTex);
	}

	void UpdateSpriteOrderAndColor(No::Registry& registry, No::Entity e, uint32_t layer, uint32_t order, const No::Color& color) {
		auto* s = registry.GetComponent<No::SpriteComponent>(e);
		if (!s) { return; }
		s->layer = layer;
		s->orderInLayer = order;
		s->color = color;
	}

	void UpdateBarVisual(
		No::Registry& registry,
		RailProgressBarComponent& bar,
		float targetRatio,
		float deltaTime
	) {
		const float follow = std::max(0.0f, bar.markerFollowSpeed);
		const float alpha = 1.0f - std::exp(-follow * std::max(0.0f, deltaTime));
		bar.displayedRatio += (targetRatio - bar.displayedRatio) * alpha;
		bar.displayedRatio = Clamp01(bar.displayedRatio);

		const float minX = std::min(bar.startPosition.x, bar.goalPosition.x);
		const float maxX = std::max(bar.startPosition.x, bar.goalPosition.x);
		const float width = std::max(1.0f, maxX - minX);
		const float fillWidth = std::max(1.0f, width * bar.displayedRatio);
		const float fillCenterX = minX + fillWidth * 0.5f;
		const float markerX = minX + width * bar.displayedRatio;

		auto* fillT = registry.GetComponent<No::Transform2DComponent>(bar.barFillEntity);
		if (fillT) {
			fillT->translate = { fillCenterX, bar.startPosition.y };
			fillT->scale = { fillWidth, std::max(1.0f, bar.barHeight) };
		}

		bar.markerPulseTime += std::max(0.0f, deltaTime) * std::max(0.0f, bar.markerPulseSpeed);
		const float pulse = 1.0f + std::sin(bar.markerPulseTime) * std::max(0.0f, bar.markerPulseScaleAmplitude);
		auto* markerT = registry.GetComponent<No::Transform2DComponent>(bar.playerMarkerEntity);
		if (markerT) {
			markerT->translate = { markerX, bar.startPosition.y };
			markerT->scale = bar.markerSize * pulse;
		}

		auto* startT = registry.GetComponent<No::Transform2DComponent>(bar.startEntity);
		if (startT) { startT->translate = bar.startPosition; }
		auto* goalT = registry.GetComponent<No::Transform2DComponent>(bar.goalEntity);
		if (goalT) { goalT->translate = bar.goalPosition; }
		auto* baseT = registry.GetComponent<No::Transform2DComponent>(bar.barBaseEntity);
		if (baseT) {
			baseT->translate = { (minX + maxX) * 0.5f, bar.startPosition.y };
			baseT->scale = { width, std::max(1.0f, bar.barHeight) };
		}
	}
}

void RailProgressBarSystem::Update(No::Registry& registry, float deltaTime)
{
	GameResourceComponent* resource = nullptr;
	for (auto e : registry.View<CBGameResourceTag, GameResourceComponent>()) {
		resource = registry.GetComponent<GameResourceComponent>(e);
		if (resource) { break; }
	}
	if (!resource) { return; }

	RailCameraComponent* rail = nullptr;
	for (auto e : registry.View<RailCameraComponent>()) {
		rail = registry.GetComponent<RailCameraComponent>(e);
		if (rail) { break; }
	}

	const NoEngine::TextureRef& startTex = GetGameTextureOrWhite(*resource, CommentBoutResourceKey::kRailProgressStart);
	const NoEngine::TextureRef& goalTex = GetGameTextureOrWhite(*resource, CommentBoutResourceKey::kRailProgressGoal);
	const NoEngine::TextureRef& baseTex = GetGameTextureOrWhite(*resource, CommentBoutResourceKey::kRailProgressBarBase);
	const NoEngine::TextureRef& fillTex = GetGameTextureOrWhite(*resource, CommentBoutResourceKey::kRailProgressBarFill);
	const NoEngine::TextureRef& markerTex = GetGameTextureOrWhite(*resource, CommentBoutResourceKey::kRailProgressPlayerMarker);

#ifdef USE_IMGUI
	ImGui::Begin("Rail Progress Bar");
#endif

	for (auto e : registry.View<CBRailProgressBarTag, RailProgressBarComponent>()) {
		auto* bar = registry.GetComponent<RailProgressBarComponent>(e);
		if (!bar) { continue; }

		EnsureSprites(registry, *bar, startTex, goalTex, baseTex, fillTex, markerTex);

#ifdef USE_IMGUI
		ImGui::PushID(static_cast<int>(e));
		if (ImGui::TreeNode("ProgressBar")) {
			ImGui::DragFloat2("Start Pos", &bar->startPosition.x, 1.0f);
			ImGui::DragFloat2("Goal Pos", &bar->goalPosition.x, 1.0f);
			ImGui::DragFloat("Bar Height", &bar->barHeight, 0.2f, 1.0f, 80.0f);
			ImGui::DragFloat2("Marker Size", &bar->markerSize.x, 0.2f, 1.0f, 120.0f);
			ImGui::DragFloat("Follow Speed", &bar->markerFollowSpeed, 0.05f, 0.0f, 30.0f);
			ImGui::DragFloat("Pulse Speed", &bar->markerPulseSpeed, 0.05f, 0.0f, 30.0f);
			ImGui::DragFloat("Pulse Amp", &bar->markerPulseScaleAmplitude, 0.01f, 0.0f, 1.0f);
			ImGui::DragInt("Layer", &bar->layer, 1.0f, 0, 200);
			ImGui::DragInt("OrderBase", &bar->orderBase, 1.0f, 0, 4000);
			ImGui::ColorEdit4("Start Color", &bar->startColor.r);
			ImGui::ColorEdit4("Goal Color", &bar->goalColor.r);
			ImGui::ColorEdit4("Base Color", &bar->barBaseColor.r);
			ImGui::ColorEdit4("Fill Color", &bar->barFillColor.r);
			ImGui::ColorEdit4("Marker Color", &bar->markerColor.r);
			if (rail && rail->totalLength > 0.0f) {
				ImGui::Text("Rail %.2f / %.2f (%.1f%%)", rail->distance, rail->totalLength, (rail->distance / rail->totalLength) * 100.0f);
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
#endif

		const uint32_t layer = static_cast<uint32_t>(std::max(0, bar->layer));

		UpdateSpriteOrderAndColor(registry, bar->barBaseEntity, layer, static_cast<uint32_t>(std::max(0, bar->orderBase + 0)), bar->barBaseColor);
		UpdateSpriteOrderAndColor(registry, bar->barFillEntity, layer, static_cast<uint32_t>(std::max(0, bar->orderBase + 1)), bar->barFillColor);
		UpdateSpriteOrderAndColor(registry, bar->startEntity, layer, static_cast<uint32_t>(std::max(0, bar->orderBase + 2)), bar->startColor);
		UpdateSpriteOrderAndColor(registry, bar->goalEntity, layer, static_cast<uint32_t>(std::max(0, bar->orderBase + 3)), bar->goalColor);
		UpdateSpriteOrderAndColor(registry, bar->playerMarkerEntity, layer, static_cast<uint32_t>(std::max(0, bar->orderBase + 4)), bar->markerColor);

		float ratio = 0.0f;
		if (rail && rail->totalLength > 0.0f) {
			ratio = Clamp01(rail->distance / rail->totalLength);
		}

		UpdateBarVisual(registry, *bar, ratio, deltaTime);
	}

#ifdef USE_IMGUI
	ImGui::End();
#endif
}
