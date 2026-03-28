#include "stdafx.h"
#include "ClearOverViewSystem.h"
#include "application/CommentBout/Component/ClearOverStateComponent.h"
#include "application/CommentBout/Component/ClearOverConfigComponent.h"
#include "application/CommentBout/Component/ClearOverViewComponent.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Utility/CBSpriteLayer.h"
#include "engine/Functions/ECS/Component/Transform2DComponent.h"
#include "engine/Functions/ECS/Component/SpriteComponent.h"
#include "engine/Runtime/GraphicsCore.h"
#include <algorithm>
#include <cmath>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4702)
#endif

namespace {
float Clamp01(float v) {
	return (v < 0.f) ? 0.f : (v > 1.f) ? 1.f : v;
}

No::Vector2 LerpVec2(const No::Vector2& a, const No::Vector2& b, float t) {
	t = Clamp01(t);
	return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t };
}

float SafeDuration(float d) {
	return (d <= 0.0001f) ? 0.0001f : d;
}

float EaseOutCubic(float t) {
	t = Clamp01(t);
	const float f = 1.f - t;
	return 1.f - f * f * f;
}

const GameResourceComponent* FindGameResource(No::Registry& registry) {
	auto gameResourceView = registry.View<CBGameResourceTag, GameResourceComponent>();
	for (auto e : gameResourceView) {
		return registry.GetComponent<GameResourceComponent>(e);
	}
	return nullptr;
}

// スプライトエンティティを生成するヘルパー
No::Entity CreateSpriteEntity(
	No::Registry& registry,
	const NoEngine::TextureRef& tex,
	const No::Vector2& pos,
	const No::Vector2& scale,
	uint32_t layer,
	float alpha = 0.f
) {
	auto e = registry.GenerateEntity();
	auto* t = registry.AddComponent<No::Transform2DComponent>(e);
	t->translate = pos;
	t->scale = scale;
	auto* sp = registry.AddComponent<No::SpriteComponent>(e);
	sp->textureHandle = tex;
	sp->layer = layer;
	sp->isVisible = true;
	sp->color = { 0.f, 0.f, 0.f, alpha };
	return e;
}

void DestroyIfValid(No::Registry& registry, No::Entity& entity) {
	if (entity != No::nullEntity) {
		registry.DestroyEntity(entity);
		entity = No::nullEntity;
	}
}

}

void ClearOverViewSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);

	ClearOverStateComponent* state = nullptr;
	ClearOverConfigComponent* config = nullptr;
	ClearOverViewComponent* view = nullptr;

	No::Entity stateEntity = No::nullEntity;
	auto stateView = registry.View<CBClearOverStateTag, ClearOverStateComponent>();
	for (auto e : stateView) {
		state = registry.GetComponent<ClearOverStateComponent>(e);
		stateEntity = e;
		break;
	}
	auto configView = registry.View<CBClearOverConfigTag, ClearOverConfigComponent>();
	for (auto e : configView) {
		config = registry.GetComponent<ClearOverConfigComponent>(e);
		break;
	}
	if (!state || !config) {
		return;
	}

	// stateEntity に ClearOverViewComponent がなければ追加
	view = registry.GetComponent<ClearOverViewComponent>(stateEntity);
	if (!view) {
		view = registry.AddComponent<ClearOverViewComponent>(stateEntity);
	}

	// ── Inactive: 全スプライトを破棄 ────────────────────────
	if (state->phase == ClearOverStateComponent::Phase::Inactive) {
		DestroyIfValid(registry, view->fadeEntity);
		DestroyIfValid(registry, view->logoEntity);
		for (auto& e : view->itemEntities) {
			DestroyIfValid(registry, e);
		}
		return;
	}

	const GameResourceComponent* gameResource = FindGameResource(registry);

	// ── FadeIn 開始時にスプライトを生成 ───────────────────────
	if (state->phase == ClearOverStateComponent::Phase::FadeIn && view->fadeEntity == No::nullEntity) {
		float screenW = 1280.f;
		float screenH = 720.f;
		if (auto* mainWindow = NoEngine::GraphicsCore::gWindowManager.GetMainWindow()) {
			const auto& ws = mainWindow->GetWindowSize();
			screenW = static_cast<float>(ws.clientWidth);
			screenH = static_cast<float>(ws.clientHeight);
		}

		// フェードオーバーレイ (黒)
		const NoEngine::TextureRef fadeTex = gameResource
			? GetGameTextureOrWhite(*gameResource, CommentBoutResourceKey::kFadeOverlay)
			: NoEngine::TextureRef{};
		view->fadeEntity = CreateSpriteEntity(
			registry, fadeTex,
			{ screenW * 0.5f, screenH * 0.5f }, { screenW, screenH },
			CommentBout::ToLayer(CommentBout::SpriteLayer::ClearOverFade), 0.f
		);

		// ロゴ
		const std::string logoKey = (state->result == ClearOverStateComponent::Result::Clear)
			? CommentBoutResourceKey::kClearLogo
			: CommentBoutResourceKey::kOverLogo;
		const NoEngine::TextureRef logoTex = gameResource
			? GetGameTextureOrWhite(*gameResource, logoKey)
			: NoEngine::TextureRef{};
		view->logoEntity = CreateSpriteEntity(
			registry, logoTex,
			config->logoStartPos, config->logoSize,
			CommentBout::ToLayer(CommentBout::SpriteLayer::ClearOverLogo), 0.f
		);

		// メニュー 2 アイテム
		const char* itemKeys[2] = {
			CommentBoutResourceKey::kRestart,
			CommentBoutResourceKey::kPauseToTitle
		};
		for (int i = 0; i < 2; ++i) {
			const NoEngine::TextureRef itemTex = gameResource
				? GetGameTextureOrWhite(*gameResource, itemKeys[i])
				: NoEngine::TextureRef{};
			const float baseY = config->itemBaseStartPos.y + config->itemSpacing * static_cast<float>(i);
			view->itemEntities[i] = CreateSpriteEntity(
				registry, itemTex,
				{ config->itemBaseStartPos.x, baseY }, config->itemSize,
				CommentBout::ToLayer(CommentBout::SpriteLayer::ClearOverItem), 0.f
			);
		}
	}

	// ── フェード更新 ─────────────────────────────────────────
	{
		float fadeAlpha = 0.f;
		if (state->phase == ClearOverStateComponent::Phase::FadeIn) {
			const float t = state->phaseTimer / SafeDuration(config->fadeDuration);
			fadeAlpha = config->fadeTargetAlpha * Clamp01(t);
		} else {
			fadeAlpha = config->fadeTargetAlpha;
		}
		if (view->fadeEntity != No::nullEntity) {
			float screenW = 1280.f;
			float screenH = 720.f;
			if (auto* mainWindow = NoEngine::GraphicsCore::gWindowManager.GetMainWindow()) {
				const auto& ws = mainWindow->GetWindowSize();
				screenW = static_cast<float>(ws.clientWidth);
				screenH = static_cast<float>(ws.clientHeight);
			}
			if (auto* t = registry.GetComponent<No::Transform2DComponent>(view->fadeEntity)) {
				t->translate = { screenW * 0.5f, screenH * 0.5f };
				t->scale = { screenW, screenH };
			}
			if (auto* sp = registry.GetComponent<No::SpriteComponent>(view->fadeEntity)) {
				sp->color = { 0.f, 0.f, 0.f, fadeAlpha };
				sp->isVisible = (fadeAlpha > 0.0001f);
			}
		}
	}

	// ── ロゴ更新 ─────────────────────────────────────────────
	{
		float logoT = 0.f;
		float logoAlpha = 0.f;
		if (state->phase == ClearOverStateComponent::Phase::LogoAppear) {
			logoT = EaseOutCubic(state->phaseTimer / SafeDuration(config->logoAppearDuration));
			logoAlpha = logoT;
		} else if (state->phase == ClearOverStateComponent::Phase::MenuAppear ||
		           state->phase == ClearOverStateComponent::Phase::MenuActive) {
			logoT = 1.f;
			logoAlpha = 1.f;
		}
		const No::Vector2 logoPos = LerpVec2(config->logoStartPos, config->logoEndPos, logoT);
		if (view->logoEntity != No::nullEntity) {
			if (auto* t = registry.GetComponent<No::Transform2DComponent>(view->logoEntity)) {
				t->translate = logoPos;
				t->scale = config->logoSize;
			}
			if (auto* sp = registry.GetComponent<No::SpriteComponent>(view->logoEntity)) {
				sp->color = { 1.f, 1.f, 1.f, logoAlpha };
				sp->isVisible = (logoAlpha > 0.0001f);
			}
		}
	}

	// ── メニュー更新 ─────────────────────────────────────────
	{
		float menuT = 0.f;
		if (state->phase == ClearOverStateComponent::Phase::MenuAppear) {
			menuT = EaseOutCubic(state->phaseTimer / SafeDuration(config->menuAppearDuration));
		} else if (state->phase == ClearOverStateComponent::Phase::MenuActive) {
			menuT = 1.f;
		}
		const float menuAlpha = menuT;
		const No::Vector2 itemBase = LerpVec2(config->itemBaseStartPos, config->itemBaseEndPos, menuT);

		float confirmPunch = 0.f;
		if (state->isConfirmAnimating) {
			const float t = state->confirmAnimTime / SafeDuration(config->confirmDuration);
			const float eased = EaseOutCubic(t);
			confirmPunch = 1.f - std::fabs(2.f * eased - 1.f);
		}

		for (int i = 0; i < 2; ++i) {
			const No::Entity itemE = view->itemEntities[i];
			if (itemE == No::nullEntity) {
				continue;
			}

			const float baseY = itemBase.y + config->itemSpacing * static_cast<float>(i);

			float itemScale = 1.f;
			const bool isSelected = (state->phase == ClearOverStateComponent::Phase::MenuActive && state->selectedIndex == i);
			if (isSelected) {
				itemScale = config->selectedScale;
			}
			if (state->isConfirmAnimating && state->confirmIndex == i) {
				itemScale = 1.f + (config->confirmScale - 1.f) * confirmPunch;
			}

			if (auto* t = registry.GetComponent<No::Transform2DComponent>(itemE)) {
				t->translate = { itemBase.x, baseY };
				t->scale = { config->itemSize.x * itemScale, config->itemSize.y * itemScale };
			}
			const float brightness = (isSelected || (state->isConfirmAnimating && state->confirmIndex == i)) ? 1.f : 0.82f;
			if (auto* sp = registry.GetComponent<No::SpriteComponent>(itemE)) {
				sp->color = { brightness, brightness, brightness, menuAlpha };
				sp->isVisible = (menuAlpha > 0.0001f);
			}
		}
	}
}
