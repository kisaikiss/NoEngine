#include "stdafx.h"
#include "PauseViewSystem.h"
#include "application/CommentBout/Component/PauseStateComponent.h"
#include "application/CommentBout/Component/PauseMenuConfigComponent.h"
#include "application/CommentBout/Component/PauseMenuViewComponent.h"
#include "application/CommentBout/GameTag.h"
#include "engine/Runtime/GraphicsCore.h"
#include <algorithm>
#include <cmath>

namespace {
	float Clamp01(float v) {
		if (v < 0.0f) {
			return 0.0f;
		}
		if (v > 1.0f) {
			return 1.0f;
		}
		return v;
	}

	No::Vector2 LerpVec2(const No::Vector2& a, const No::Vector2& b, float t) {
		t = Clamp01(t);
		return No::Vector2(
			a.x + (b.x - a.x) * t,
			a.y + (b.y - a.y) * t
		);
	}

	float SafeDuration(float d) {
		return (d <= 0.0001f) ? 0.0001f : d;
	}

	float EaseByType(int easeType, float t) {
		t = Clamp01(t);
		switch (easeType) {
		case 1:
			return No::EaseInExpo(0.0f, 1.0f, t);
		case 2:
			return No::EaseOutCubic(0.0f, 1.0f, t);
		default:
			return No::EaseInOutSine(0.0f, 1.0f, t);
		}
	}

	float GetMenuMotionT(const PauseStateComponent* pauseState) {
		if (!pauseState) {
			return 0.0f;
		}
		const float duration = (pauseState->phaseDuration <= 0.0001f) ? 0.0001f : pauseState->phaseDuration;
		switch (pauseState->phase) {
		case PauseStateComponent::Opening:
		{
			const float t = pauseState->phaseTime / duration;
			return No::EaseOutBack(0.0f, 1.0f, t);
		}
		case PauseStateComponent::Closing:
		{
			const float t = pauseState->phaseTime / duration;
			return 1.0f - No::EaseOutCubic(0.0f, 1.0f, t);
		}
		case PauseStateComponent::Open:
		case PauseStateComponent::OptionOpening:
		case PauseStateComponent::OptionOpen:
		case PauseStateComponent::OptionClosing:
			return 1.0f;
		case PauseStateComponent::Closed:
		default:
			return 0.0f;
		}
	}
}

void PauseViewSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);
	PauseStateComponent* pauseState = nullptr;
	PauseMenuConfigComponent* pauseConfig = nullptr;
	PauseMenuViewComponent* pauseView = nullptr;

	auto pauseStateView = registry.View<CBPauseStateTag, PauseStateComponent>();
	for (auto entity : pauseStateView) {
		pauseState = registry.GetComponent<PauseStateComponent>(entity);
		if (pauseState) {
			break;
		}
	}

	auto pauseConfigView = registry.View<CBPauseConfigTag, PauseMenuConfigComponent>();
	for (auto entity : pauseConfigView) {
		pauseConfig = registry.GetComponent<PauseMenuConfigComponent>(entity);
		if (pauseConfig) {
			break;
		}
	}

	auto pauseUiView = registry.View<CBPauseViewTag, PauseMenuViewComponent>();
	for (auto entity : pauseUiView) {
		pauseView = registry.GetComponent<PauseMenuViewComponent>(entity);
		if (pauseView) {
			break;
		}
	}

	if (!pauseState || !pauseConfig || !pauseView) {
		return;
	}

	if (pauseView->dimEntity != No::nullEntity &&
		registry.Has<No::Transform2DComponent>(pauseView->dimEntity) &&
		registry.Has<No::SpriteComponent>(pauseView->dimEntity)) {
		auto* dimTransform = registry.GetComponent<No::Transform2DComponent>(pauseView->dimEntity);
		auto* dimSprite = registry.GetComponent<No::SpriteComponent>(pauseView->dimEntity);

		auto* mainWindow = NoEngine::GraphicsCore::gWindowManager.GetMainWindow();
		if (mainWindow) {
			const auto& windowSize = mainWindow->GetWindowSize();
			const float width = static_cast<float>(windowSize.clientWidth);
			const float height = static_cast<float>(windowSize.clientHeight);
			dimTransform->translate = { width * 0.5f, height * 0.5f };
			dimTransform->scale = { width, height };
		}

		const float maxAlpha = std::max(0.0f, std::min(1.0f, pauseConfig->dimAlpha));
		float alpha = 0.0f;

		switch (pauseState->phase) {
		case PauseStateComponent::Opening:
		{
			const float t = pauseState->phaseTime / SafeDuration(pauseState->phaseDuration);
			alpha = maxAlpha * EaseByType(pauseConfig->easeType, t);
		}
		break;
		case PauseStateComponent::Closing:
		{
			const float t = pauseState->phaseTime / SafeDuration(pauseState->phaseDuration);
			alpha = maxAlpha * (1.0f - EaseByType(pauseConfig->easeType, t));
		}
		break;
		case PauseStateComponent::OptionOpening:
		{
			const float t = pauseState->phaseTime / SafeDuration(pauseState->phaseDuration);
			alpha = maxAlpha + 0.12f * EaseByType(pauseConfig->easeType, t);
		}
		break;
		case PauseStateComponent::OptionOpen:
			alpha = maxAlpha + 0.12f;
			break;
		case PauseStateComponent::OptionClosing:
		{
			const float t = pauseState->phaseTime / SafeDuration(pauseState->phaseDuration);
			alpha = (maxAlpha + 0.12f) - 0.12f * EaseByType(pauseConfig->easeType, t);
		}
		break;
		case PauseStateComponent::Open:
			alpha = maxAlpha;
			break;
		case PauseStateComponent::Closed:
		default:
			alpha = 0.0f;
			break;
		}

		alpha = std::max(0.0f, std::min(1.0f, alpha));
		dimSprite->layer = static_cast<uint32_t>(std::max(0, pauseConfig->dimLayer));
		dimSprite->isVisible = (alpha > 0.0001f);
		dimSprite->color = { 0.0f, 0.0f, 0.0f, alpha };
	}

	const float menuMotionT = Clamp01(GetMenuMotionT(pauseState));
	const float menuVisibility = menuMotionT;

	float confirmPunch = 0.0f;
	if (pauseState->isConfirmAnimating) {
		const float t = pauseState->confirmAnimTime / SafeDuration(pauseConfig->confirmDuration);
		const float eased = EaseByType(pauseConfig->easeType, t);
		confirmPunch = 1.0f - std::fabs(2.0f * eased - 1.0f);
	}

	if (pauseView->menuBgEntity != No::nullEntity &&
		registry.Has<No::Transform2DComponent>(pauseView->menuBgEntity) &&
		registry.Has<No::SpriteComponent>(pauseView->menuBgEntity)) {
		auto* bgTransform = registry.GetComponent<No::Transform2DComponent>(pauseView->menuBgEntity);
		auto* bgSprite = registry.GetComponent<No::SpriteComponent>(pauseView->menuBgEntity);
		bgTransform->translate = LerpVec2(pauseConfig->menuBgStartPosition, pauseConfig->menuBgEndPosition, menuMotionT);
		bgTransform->scale = LerpVec2(pauseConfig->menuBgStartSize, pauseConfig->menuBgEndSize, menuMotionT);
		bgSprite->layer = static_cast<uint32_t>(std::max(0, pauseConfig->menuBgLayer));
		bgSprite->isVisible = (menuVisibility > 0.0001f);
		bgSprite->color = {
			pauseConfig->menuBgColor.r,
			pauseConfig->menuBgColor.g,
			pauseConfig->menuBgColor.b,
			pauseConfig->menuBgColor.a * menuVisibility
		};
	}

	if (pauseView->panelLineEntity != No::nullEntity &&
		registry.Has<No::Transform2DComponent>(pauseView->panelLineEntity) &&
		registry.Has<No::SpriteComponent>(pauseView->panelLineEntity)) {
		auto* lineTransform = registry.GetComponent<No::Transform2DComponent>(pauseView->panelLineEntity);
		auto* lineSprite = registry.GetComponent<No::SpriteComponent>(pauseView->panelLineEntity);
		lineTransform->translate = LerpVec2(pauseConfig->panelLineStartPosition, pauseConfig->panelLineEndPosition, menuMotionT);
		lineTransform->scale = LerpVec2(pauseConfig->panelLineStartSize, pauseConfig->panelLineEndSize, menuMotionT);
		lineSprite->layer = static_cast<uint32_t>(std::max(0, pauseConfig->panelLineLayer));
		lineSprite->isVisible = (menuVisibility > 0.0001f);
		lineSprite->color = {
			pauseConfig->panelLineColor.r,
			pauseConfig->panelLineColor.g,
			pauseConfig->panelLineColor.b,
			pauseConfig->panelLineColor.a * menuVisibility
		};
	}

	if (pauseView->titleEntity != No::nullEntity &&
		registry.Has<No::Transform2DComponent>(pauseView->titleEntity) &&
		registry.Has<No::SpriteComponent>(pauseView->titleEntity)) {
		auto* titleTransform = registry.GetComponent<No::Transform2DComponent>(pauseView->titleEntity);
		auto* titleSprite = registry.GetComponent<No::SpriteComponent>(pauseView->titleEntity);
		titleTransform->translate = LerpVec2(pauseConfig->titleStartPosition, pauseConfig->titleEndPosition, menuMotionT);
		titleTransform->scale = pauseConfig->titleSize;
		titleSprite->layer = static_cast<uint32_t>(std::max(0, pauseConfig->titleLayer));
		titleSprite->isVisible = (menuVisibility > 0.0001f);
		titleSprite->color = { 1.0f, 1.0f, 1.0f, menuVisibility };
	}

	const No::Vector2 itemBasePosition = LerpVec2(pauseConfig->itemBaseStartPosition, pauseConfig->itemBaseEndPosition, menuMotionT);
	for (size_t i = 0; i < pauseView->itemEntities.size(); ++i) {
		const No::Entity e = pauseView->itemEntities[i];
		if (e == No::nullEntity) {
			continue;
		}
		if (!registry.Has<No::Transform2DComponent>(e) || !registry.Has<No::SpriteComponent>(e)) {
			continue;
		}

		auto* itemTransform = registry.GetComponent<No::Transform2DComponent>(e);
		auto* itemSprite = registry.GetComponent<No::SpriteComponent>(e);

		const float baseY = itemBasePosition.y + pauseConfig->itemSpacing * static_cast<float>(i);

		float itemScale = 1.0f;
		const bool isSelected = (static_cast<int>(i) == pauseState->selectedIndex && pauseState->phase == PauseStateComponent::Open);
		if (isSelected) {
			itemScale = pauseConfig->selectedScale;
		}
		if (pauseState->isConfirmAnimating && static_cast<int>(i) == pauseState->confirmIndex) {
			itemScale = 1.0f + (pauseConfig->confirmScale - 1.0f) * confirmPunch;
		}

		itemTransform->translate = { itemBasePosition.x, baseY };
		itemTransform->scale = {
			pauseConfig->itemSize.x * itemScale,
			pauseConfig->itemSize.y * itemScale
		};

		itemSprite->layer = static_cast<uint32_t>(std::max(0, pauseConfig->itemLayer));
		itemSprite->orderInLayer = static_cast<uint32_t>(i);
		itemSprite->isVisible = (menuVisibility > 0.0001f);
		if (isSelected || (pauseState->isConfirmAnimating && static_cast<int>(i) == pauseState->confirmIndex)) {
			itemSprite->color = { 1.0f, 1.0f, 1.0f, menuVisibility };
		}
		else {
			itemSprite->color = { 0.82f, 0.82f, 0.82f, menuVisibility };
		}
	}

	if (pauseView->cursorEntity != No::nullEntity &&
		registry.Has<No::Transform2DComponent>(pauseView->cursorEntity) &&
		registry.Has<No::SpriteComponent>(pauseView->cursorEntity)) {
		auto* cursorTransform = registry.GetComponent<No::Transform2DComponent>(pauseView->cursorEntity);
		auto* cursorSprite = registry.GetComponent<No::SpriteComponent>(pauseView->cursorEntity);
		const float cursorY = itemBasePosition.y + pauseConfig->itemSpacing * static_cast<float>(pauseState->selectedIndex);
		const No::Vector2 cursorOffset = LerpVec2(pauseConfig->cursorStartOffset, pauseConfig->cursorEndOffset, menuMotionT);
		cursorTransform->translate = {
			itemBasePosition.x + cursorOffset.x,
			cursorY + cursorOffset.y
		};
		cursorTransform->scale = pauseConfig->cursorSize;
		cursorSprite->layer = static_cast<uint32_t>(std::max(0, pauseConfig->cursorLayer));
		cursorSprite->isVisible = (!pauseState->isConfirmAnimating && pauseState->phase == PauseStateComponent::Open && menuVisibility > 0.0001f);
		cursorSprite->color = { 1.0f, 0.95f, 0.35f, menuVisibility };
	}
}
