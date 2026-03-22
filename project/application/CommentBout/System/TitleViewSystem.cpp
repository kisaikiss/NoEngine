#include "stdafx.h"
#include "TitleViewSystem.h"
#include "application/CommentBout/Component/TitleMenuStateComponent.h"
#include "application/CommentBout/Component/TitleMenuConfigComponent.h"
#include "application/CommentBout/Component/TitleMenuViewComponent.h"
#include "application/CommentBout/Component/OptionStateComponent.h"
#include "application/CommentBout/GameTag.h"
#include <algorithm>
#include <cmath>

namespace {
	float Clamp01(float v) {
		if (v < 0.0f) return 0.0f;
		if (v > 1.0f) return 1.0f;
		return v;
	}

	float SafeDuration(float d) {
		return (d <= 0.0001f) ? 0.0001f : d;
	}
}

void TitleViewSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);
	TitleMenuStateComponent* titleState = nullptr;
	TitleMenuConfigComponent* titleConfig = nullptr;
	TitleMenuViewComponent* titleView = nullptr;
	OptionStateComponent* optionState = nullptr;

	auto stateView = registry.View<CBTitleStateTag, TitleMenuStateComponent>();
	for (auto entity : stateView) {
		titleState = registry.GetComponent<TitleMenuStateComponent>(entity);
		if (titleState) {
			break;
		}
	}

	auto configView = registry.View<CBTitleConfigTag, TitleMenuConfigComponent>();
	for (auto entity : configView) {
		titleConfig = registry.GetComponent<TitleMenuConfigComponent>(entity);
		if (titleConfig) {
			break;
		}
	}

	auto uiView = registry.View<CBTitleViewTag, TitleMenuViewComponent>();
	for (auto entity : uiView) {
		titleView = registry.GetComponent<TitleMenuViewComponent>(entity);
		if (titleView) {
			break;
		}
	}

	auto optionView = registry.View<CBOptionStateTag, OptionStateComponent>();
	for (auto entity : optionView) {
		optionState = registry.GetComponent<OptionStateComponent>(entity);
		if (optionState) {
			break;
		}
	}

	if (!titleState || !titleConfig || !titleView) {
		return;
	}

	float confirmPunch = 0.0f;
	if (titleState->isConfirmAnimating) {
		const float t = Clamp01(titleState->confirmAnimTime / SafeDuration(titleConfig->confirmDuration));
		const float eased = No::EaseOutCubic(0.0f, 1.0f, t);
		confirmPunch = 1.0f - std::fabs(2.0f * eased - 1.0f);
	}

	if (titleView->backgroundEntity != No::nullEntity &&
		registry.Has<No::Transform2DComponent>(titleView->backgroundEntity) &&
		registry.Has<No::SpriteComponent>(titleView->backgroundEntity)) {
		auto* tr = registry.GetComponent<No::Transform2DComponent>(titleView->backgroundEntity);
		auto* sp = registry.GetComponent<No::SpriteComponent>(titleView->backgroundEntity);
		tr->translate = titleConfig->backgroundPosition;
		tr->scale = titleConfig->backgroundSize;
		tr->rotation = titleConfig->backgroundRotation;
		sp->layer = static_cast<uint32_t>(std::max(0, titleConfig->backgroundLayer));
		sp->isVisible = true;
		sp->color = titleConfig->backgroundColor;
	}

	if (titleView->panelEntity != No::nullEntity &&
		registry.Has<No::Transform2DComponent>(titleView->panelEntity) &&
		registry.Has<No::SpriteComponent>(titleView->panelEntity)) {
		auto* tr = registry.GetComponent<No::Transform2DComponent>(titleView->panelEntity);
		auto* sp = registry.GetComponent<No::SpriteComponent>(titleView->panelEntity);
		tr->translate = titleConfig->panelPosition;
		tr->scale = titleConfig->panelSize;
		tr->rotation = titleConfig->panelRotation;
		sp->layer = static_cast<uint32_t>(std::max(0, titleConfig->panelLayer));
		sp->isVisible = true;
		sp->color = titleConfig->panelColor;
	}

	if (titleView->logoEntity != No::nullEntity &&
		registry.Has<No::Transform2DComponent>(titleView->logoEntity) &&
		registry.Has<No::SpriteComponent>(titleView->logoEntity)) {
		auto* tr = registry.GetComponent<No::Transform2DComponent>(titleView->logoEntity);
		auto* sp = registry.GetComponent<No::SpriteComponent>(titleView->logoEntity);

		const float t = titleState->logoMotionTime;
		const float posOffset = std::sinf(t * titleConfig->logoMotionPosSpeed) * titleConfig->logoMotionPosAmplitude;
		const float scaleOffset = std::sinf(t * titleConfig->logoMotionScaleSpeed) * titleConfig->logoMotionScaleAmplitude;
		const float rotationOffset = std::sinf(t * titleConfig->logoMotionRotateSpeed) * titleConfig->logoMotionRotateAmplitude;

		tr->translate = {
			titleConfig->logoPosition.x,
			titleConfig->logoPosition.y + posOffset
		};
		tr->scale = {
			titleConfig->logoSize.x * (1.0f + scaleOffset),
			titleConfig->logoSize.y * (1.0f + scaleOffset)
		};
		tr->rotation = titleConfig->logoBaseRotation + rotationOffset;
		sp->layer = static_cast<uint32_t>(std::max(0, titleConfig->logoLayer));
		sp->isVisible = true;
		sp->color = No::Color::WHITE;
	}

	for (size_t i = 0; i < titleView->itemEntities.size(); ++i) {
		const auto itemEntity = titleView->itemEntities[i];
		if (itemEntity == No::nullEntity ||
			!registry.Has<No::Transform2DComponent>(itemEntity) ||
			!registry.Has<No::SpriteComponent>(itemEntity)) {
			continue;
		}

		auto* tr = registry.GetComponent<No::Transform2DComponent>(itemEntity);
		auto* sp = registry.GetComponent<No::SpriteComponent>(itemEntity);

		const bool isSelected = (static_cast<int>(i) == titleState->selectedIndex);
		float scale = 1.0f;
		if (isSelected) {
			scale = titleConfig->selectedScale;
		}
		if (titleState->isConfirmAnimating && static_cast<int>(i) == titleState->confirmIndex) {
			scale = 1.0f + (titleConfig->confirmScale - 1.0f) * confirmPunch;
		}

		tr->translate = {
			titleConfig->itemBasePosition.x,
			titleConfig->itemBasePosition.y + titleConfig->itemSpacing * static_cast<float>(i)
		};
		tr->scale = {
			titleConfig->itemSize.x * scale,
			titleConfig->itemSize.y * scale
		};
		sp->layer = static_cast<uint32_t>(std::max(0, titleConfig->itemLayer));
		sp->orderInLayer = static_cast<uint32_t>(i);
		sp->isVisible = true;
		sp->color = isSelected ? titleConfig->selectedItemColor : titleConfig->itemColor;
	}

	if (titleView->cursorEntity != No::nullEntity &&
		registry.Has<No::Transform2DComponent>(titleView->cursorEntity) &&
		registry.Has<No::SpriteComponent>(titleView->cursorEntity)) {
		auto* tr = registry.GetComponent<No::Transform2DComponent>(titleView->cursorEntity);
		auto* sp = registry.GetComponent<No::SpriteComponent>(titleView->cursorEntity);
		const float cursorY = titleConfig->itemBasePosition.y + titleConfig->itemSpacing * static_cast<float>(titleState->selectedIndex);
		tr->translate = {
			titleConfig->itemBasePosition.x + titleConfig->cursorOffset.x,
			cursorY + titleConfig->cursorOffset.y
		};
		tr->scale = titleConfig->cursorSize;
		sp->layer = static_cast<uint32_t>(std::max(0, titleConfig->cursorLayer));
		sp->isVisible = !titleState->isConfirmAnimating && !(optionState && optionState->isOpen);
		sp->color = titleConfig->cursorColor;
	}
}
