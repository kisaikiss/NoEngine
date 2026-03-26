#include "stdafx.h"
#include "OptionViewSystem.h"
#include "application/CommentBout/Component/OutGame/OptionStateComponent.h"
#include "application/CommentBout/Component/OutGame/OptionMenuConfigComponent.h"
#include "application/CommentBout/Component/OutGame/OptionMenuViewComponent.h"
#include "application/CommentBout/GameTag.h"
#include <array>
#include <algorithm>
#include <cmath>

namespace {
	float Clamp01(float v) {
		if (v < 0.0f) return 0.0f;
		if (v > 1.0f) return 1.0f;
		return v;
	}

	No::Vector2 LerpVec2(const No::Vector2& a, const No::Vector2& b, float t) {
		t = Clamp01(t);
		return No::Vector2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
	}

	float SafeDuration(float d) {
		return (d <= 0.0001f) ? 0.0001f : d;
	}
}

void OptionViewSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);
	OptionStateComponent* optionState = nullptr;
	OptionMenuConfigComponent* optionConfig = nullptr;
	OptionMenuViewComponent* optionView = nullptr;

	auto stateView = registry.View<CBOptionStateTag, OptionStateComponent>();
	for (auto entity : stateView) {
		optionState = registry.GetComponent<OptionStateComponent>(entity);
		if (optionState) break;
	}
	auto configView = registry.View<CBOptionConfigTag, OptionMenuConfigComponent>();
	for (auto entity : configView) {
		optionConfig = registry.GetComponent<OptionMenuConfigComponent>(entity);
		if (optionConfig) break;
	}
	auto uiView = registry.View<CBOptionViewTag, OptionMenuViewComponent>();
	for (auto entity : uiView) {
		optionView = registry.GetComponent<OptionMenuViewComponent>(entity);
		if (optionView) break;
	}
	if (!optionState || !optionConfig || !optionView) return;

	// ---- アニメーション t 値 -----------------------------------------------
	float t = 0.0f;
	if (optionState->phase == OptionStateComponent::Opening) {
		t = No::EaseOutBack(0.0f, 1.0f, optionState->phaseTime / SafeDuration(optionState->phaseDuration));
	} else if (optionState->phase == OptionStateComponent::Closing) {
		t = 1.0f - No::EaseOutCubic(0.0f, 1.0f, optionState->phaseTime / SafeDuration(optionState->phaseDuration));
	} else if (optionState->phase == OptionStateComponent::OpenSelect ||
		optionState->phase == OptionStateComponent::OpenEdit) {
		t = 1.0f;
	}
	t = Clamp01(t);

	// ---- 決定アニメーション punch ------------------------------------------
	float confirmPunch = 0.0f;
	if (optionState->isConfirmAnimating) {
		const float tt = optionState->confirmAnimTime / SafeDuration(optionConfig->confirmDuration);
		const float eased = No::EaseOutCubic(0.0f, 1.0f, tt);
		confirmPunch = 1.0f - std::fabs(2.0f * eased - 1.0f);
	}

	const No::Vector2 basePos = LerpVec2(optionConfig->itemBaseStartPosition, optionConfig->itemBaseEndPosition, t);
	const No::Vector2 backItemPos = LerpVec2(optionConfig->backItemStartPosition, optionConfig->backItemEndPosition, t);

	// ---- Dim ---------------------------------------------------------------
	if (optionView->dimEntity != No::nullEntity &&
		registry.Has<No::Transform2DComponent>(optionView->dimEntity) &&
		registry.Has<No::SpriteComponent>(optionView->dimEntity))
	{
		auto* tr = registry.GetComponent<No::Transform2DComponent>(optionView->dimEntity);
		auto* sp = registry.GetComponent<No::SpriteComponent>(optionView->dimEntity);
		tr->translate = LerpVec2(optionConfig->dimStartPosition, optionConfig->dimEndPosition, t);
		tr->scale = optionConfig->dimSize;
		tr->rotation = optionConfig->dimRotation;
		sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->dimLayer));
		sp->isVisible = (t > 0.0001f);
		sp->color = { optionConfig->dimColor.r, optionConfig->dimColor.g, optionConfig->dimColor.b, optionConfig->dimColor.a * t };
	}

	// ---- Background --------------------------------------------------------
	if (optionView->bgEntity != No::nullEntity &&
		registry.Has<No::Transform2DComponent>(optionView->bgEntity) &&
		registry.Has<No::SpriteComponent>(optionView->bgEntity))
	{
		auto* tr = registry.GetComponent<No::Transform2DComponent>(optionView->bgEntity);
		auto* sp = registry.GetComponent<No::SpriteComponent>(optionView->bgEntity);
		tr->translate = LerpVec2(optionConfig->bgStartPosition, optionConfig->bgEndPosition, t);
		tr->scale = LerpVec2(optionConfig->bgStartSize, optionConfig->bgEndSize, t);
		tr->rotation = optionConfig->bgRotation;
		sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->bgLayer));
		sp->isVisible = (t > 0.0001f);
		sp->color = { optionConfig->bgColor.r, optionConfig->bgColor.g, optionConfig->bgColor.b, optionConfig->bgColor.a * t };
	}

	// ---- Separator line ----------------------------------------------------
	if (optionView->lineEntity != No::nullEntity &&
		registry.Has<No::Transform2DComponent>(optionView->lineEntity) &&
		registry.Has<No::SpriteComponent>(optionView->lineEntity))
	{
		auto* tr = registry.GetComponent<No::Transform2DComponent>(optionView->lineEntity);
		auto* sp = registry.GetComponent<No::SpriteComponent>(optionView->lineEntity);
		tr->translate = LerpVec2(optionConfig->lineStartPosition, optionConfig->lineEndPosition, t);
		tr->scale = optionConfig->lineSize;
		tr->rotation = optionConfig->lineRotation;
		sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->lineLayer));
		sp->isVisible = (t > 0.0001f);
		sp->color = { optionConfig->lineColor.r, optionConfig->lineColor.g, optionConfig->lineColor.b, optionConfig->lineColor.a * t };
	}

	// ---- Title -------------------------------------------------------------
	if (optionView->titleEntity != No::nullEntity &&
		registry.Has<No::Transform2DComponent>(optionView->titleEntity) &&
		registry.Has<No::SpriteComponent>(optionView->titleEntity))
	{
		auto* tr = registry.GetComponent<No::Transform2DComponent>(optionView->titleEntity);
		auto* sp = registry.GetComponent<No::SpriteComponent>(optionView->titleEntity);
		tr->translate = LerpVec2(optionConfig->titleStartPosition, optionConfig->titleEndPosition, t);
		tr->scale = optionConfig->titleSize;
		tr->rotation = optionConfig->titleRotation;  // ★ rotation 追加
		sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->labelLayer));
		sp->isVisible = (t > 0.0001f);
		sp->color = { 1.0f, 1.0f, 1.0f, t };
	}

	// ---- Items / Labels / Bars / Overlays ----------------------------------
	const std::array<float, 3> volumes = {
		optionState->masterVolume, optionState->bgmVolume, optionState->seVolume
	};
	const bool inEdit = (optionState->phase == OptionStateComponent::OpenEdit);

	for (size_t i = 0; i < optionView->itemEntities.size(); ++i) {
		const bool isBack = (i == 4);
		const No::Vector2 baseTranslate = isBack
			? backItemPos
			: No::Vector2(basePos.x, basePos.y + optionConfig->itemSpacing * static_cast<float>(i));
		const No::Vector2 baseSize = isBack ? optionConfig->backItemSize : optionConfig->itemSize;

		// アイテム背景
		const No::Entity itemE = optionView->itemEntities[i];
		if (itemE != No::nullEntity &&
			registry.Has<No::Transform2DComponent>(itemE) &&
			registry.Has<No::SpriteComponent>(itemE))
		{
			auto* tr = registry.GetComponent<No::Transform2DComponent>(itemE);
			auto* sp = registry.GetComponent<No::SpriteComponent>(itemE);
			float scale = 1.0f;
			if (optionState->isConfirmAnimating && static_cast<int>(i) == optionState->confirmIndex) {
				scale = 1.0f + (optionConfig->confirmScale - 1.0f) * confirmPunch;
			}
			tr->translate = baseTranslate;
			tr->scale = { baseSize.x * scale, baseSize.y * scale };
			sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->itemLayer));
			sp->orderInLayer = static_cast<uint32_t>(i);
			sp->isVisible = (t > 0.0001f);
			sp->color = { optionConfig->itemColor.r, optionConfig->itemColor.g, optionConfig->itemColor.b, optionConfig->itemColor.a * t };
		}

		// ラベル
		const No::Entity labelE = optionView->labelEntities[i];
		if (labelE != No::nullEntity &&
			registry.Has<No::Transform2DComponent>(labelE) &&
			registry.Has<No::SpriteComponent>(labelE))
		{
			auto* tr = registry.GetComponent<No::Transform2DComponent>(labelE);
			auto* sp = registry.GetComponent<No::SpriteComponent>(labelE);
			if (isBack) {
				tr->translate = LerpVec2(optionConfig->backLabelStartPosition, optionConfig->backLabelEndPosition, t);
				tr->scale = optionConfig->backLabelSize;
			} else {
				tr->translate = { basePos.x + optionConfig->labelOffset.x, basePos.y + optionConfig->itemSpacing * static_cast<float>(i) + optionConfig->labelOffset.y };
				tr->scale = optionConfig->labelSize;
			}
			sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->labelLayer));
			sp->isVisible = (t > 0.0001f);
			sp->color = { 1.0f, 1.0f, 1.0f, t };
		}

		// 音量バー（index 0〜2 のみ）
		if (i < optionView->barBaseEntities.size()) {
			const float rowY = basePos.y + optionConfig->itemSpacing * static_cast<float>(i);
			const No::Entity baseE = optionView->barBaseEntities[i];
			const No::Entity fillE = optionView->barFillEntities[i];
			if (baseE != No::nullEntity &&
				registry.Has<No::Transform2DComponent>(baseE) &&
				registry.Has<No::SpriteComponent>(baseE))
			{
				auto* tr = registry.GetComponent<No::Transform2DComponent>(baseE);
				auto* sp = registry.GetComponent<No::SpriteComponent>(baseE);
				tr->translate = { basePos.x + optionConfig->barOffset.x, rowY + optionConfig->barOffset.y };
				tr->scale = optionConfig->barBaseSize;
				sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->barBaseLayer));
				sp->isVisible = (t > 0.0001f);
				sp->color = { optionConfig->barBaseColor.r, optionConfig->barBaseColor.g, optionConfig->barBaseColor.b, optionConfig->barBaseColor.a * t };
			}
			if (fillE != No::nullEntity &&
				registry.Has<No::Transform2DComponent>(fillE) &&
				registry.Has<No::SpriteComponent>(fillE))
			{
				auto* tr = registry.GetComponent<No::Transform2DComponent>(fillE);
				auto* sp = registry.GetComponent<No::SpriteComponent>(fillE);
				const float fillW = optionConfig->barFillMinSize.x +
					(optionConfig->barBaseSize.x - optionConfig->barFillMinSize.x) * Clamp01(volumes[i]);
				tr->translate = { basePos.x + optionConfig->barOffset.x - (optionConfig->barBaseSize.x - fillW) * 0.5f, rowY + optionConfig->barOffset.y };
				tr->scale = { fillW, optionConfig->barFillMinSize.y };
				sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->barFillLayer));
				sp->isVisible = (t > 0.0001f);
				sp->color = { optionConfig->barFillColor.r, optionConfig->barFillColor.g, optionConfig->barFillColor.b, optionConfig->barFillColor.a * t };
			}
		}

		// ★ 編集モード 行暗幕オーバーレイ
		// OpenEdit 時に選択していない行を暗くして「今ここだけ操作中」を伝える
		const No::Entity overlayE = optionView->itemOverlayEntities[i];
		if (overlayE != No::nullEntity &&
			registry.Has<No::Transform2DComponent>(overlayE) &&
			registry.Has<No::SpriteComponent>(overlayE))
		{
			auto* tr = registry.GetComponent<No::Transform2DComponent>(overlayE);
			auto* sp = registry.GetComponent<No::SpriteComponent>(overlayE);
			tr->translate = baseTranslate;
			tr->scale = baseSize;
			sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->itemOverlayLayer));
			const bool isSelected = (static_cast<int>(i) == optionState->selectedIndex);
			// 選択行には出さない / 編集中のみ表示
			sp->isVisible = inEdit && !isSelected && (t > 0.0001f);
			sp->color = {
				optionConfig->itemOverlayColor.r,
				optionConfig->itemOverlayColor.g,
				optionConfig->itemOverlayColor.b,
				optionConfig->itemOverlayColor.a * t
			};
		}
	}

	// ---- Toggle ------------------------------------------------------------
	const float toggleY = basePos.y + optionConfig->itemSpacing * 3.0f;

	if (optionView->toggleEntity != No::nullEntity &&
		registry.Has<No::Transform2DComponent>(optionView->toggleEntity) &&
		registry.Has<No::SpriteComponent>(optionView->toggleEntity))
	{
		auto* tr = registry.GetComponent<No::Transform2DComponent>(optionView->toggleEntity);
		auto* sp = registry.GetComponent<No::SpriteComponent>(optionView->toggleEntity);
		tr->translate = { basePos.x + optionConfig->toggleOffset.x, toggleY + optionConfig->toggleOffset.y };
		tr->scale = optionConfig->toggleSize;
		sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->toggleLayer));
		sp->isVisible = (t > 0.0001f);
		sp->color = { optionConfig->itemColor.r, optionConfig->itemColor.g, optionConfig->itemColor.b, 0.35f * t };
	}
	if (optionView->toggleOnEntity != No::nullEntity &&
		registry.Has<No::Transform2DComponent>(optionView->toggleOnEntity) &&
		registry.Has<No::SpriteComponent>(optionView->toggleOnEntity))
	{
		auto* tr = registry.GetComponent<No::Transform2DComponent>(optionView->toggleOnEntity);
		auto* sp = registry.GetComponent<No::SpriteComponent>(optionView->toggleOnEntity);
		tr->translate = { basePos.x + optionConfig->toggleOffset.x, toggleY + optionConfig->toggleOffset.y };
		tr->scale = optionConfig->toggleSize;
		sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->toggleLayer));
		sp->isVisible = (t > 0.0001f) && optionState->vibrationEnabled;
		sp->color = { optionConfig->toggleOnColor.r, optionConfig->toggleOnColor.g, optionConfig->toggleOnColor.b, optionConfig->toggleOnColor.a * t };
	}
	if (optionView->toggleOffEntity != No::nullEntity &&
		registry.Has<No::Transform2DComponent>(optionView->toggleOffEntity) &&
		registry.Has<No::SpriteComponent>(optionView->toggleOffEntity))
	{
		auto* tr = registry.GetComponent<No::Transform2DComponent>(optionView->toggleOffEntity);
		auto* sp = registry.GetComponent<No::SpriteComponent>(optionView->toggleOffEntity);
		tr->translate = { basePos.x + optionConfig->toggleOffset.x, toggleY + optionConfig->toggleOffset.y };
		tr->scale = optionConfig->toggleSize;
		sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->toggleLayer));
		sp->isVisible = (t > 0.0001f) && !optionState->vibrationEnabled;
		sp->color = { optionConfig->toggleOffColor.r, optionConfig->toggleOffColor.g, optionConfig->toggleOffColor.b, optionConfig->toggleOffColor.a * t };
	}

	// ---- Cursor ------------------------------------------------------------
	if (optionView->cursorEntity != No::nullEntity &&
		registry.Has<No::Transform2DComponent>(optionView->cursorEntity) &&
		registry.Has<No::SpriteComponent>(optionView->cursorEntity))
	{
		auto* tr = registry.GetComponent<No::Transform2DComponent>(optionView->cursorEntity);
		auto* sp = registry.GetComponent<No::SpriteComponent>(optionView->cursorEntity);
		const float rowY = basePos.y + optionConfig->itemSpacing * static_cast<float>(optionState->selectedIndex);
		const bool isBackSel = (optionState->selectedIndex == 4);
		const No::Vector2 offset = isBackSel ? optionConfig->cursorBackOffset : optionConfig->cursorSelectOffset;
		const No::Vector2 anchor = isBackSel ? backItemPos : No::Vector2(basePos.x, rowY);
		tr->translate = { anchor.x + offset.x, anchor.y + offset.y };
		tr->scale = optionConfig->cursorSize;
		sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->cursorLayer));
		sp->isVisible = (t > 0.0001f) &&
			(optionState->phase == OptionStateComponent::OpenSelect ||
				optionState->phase == OptionStateComponent::OpenEdit);
		const No::Color cc = (optionState->phase == OptionStateComponent::OpenEdit)
			? optionConfig->cursorEditColor
			: optionConfig->cursorColor;
		sp->color = { cc.r, cc.g, cc.b, cc.a * t };
	}
}