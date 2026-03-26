#include "stdafx.h"
#include "PauseMenuSpawner.h"
#include "application/CommentBout/Utility/CBSpriteLayer.h"
#include "application/CommentBout/Component/OutGame/PauseMenuViewComponent.h"
#include "application/CommentBout/GameTag.h"

void PauseMenuSpawner::Create(No::Registry& registry, const GameResourceComponent& resources)
{
	auto pauseViewEntity = registry.GenerateEntity();
	registry.AddComponent<CBPauseViewTag>(pauseViewEntity);
	auto* pauseView = registry.AddComponent<PauseMenuViewComponent>(pauseViewEntity);

	const auto& whiteTexture = GetGameTextureOrWhite(resources, CommentBoutResourceKey::kWhiteTexture);
	const auto& pauseTitleTexture = GetGameTextureOrWhite(resources, CommentBoutResourceKey::kPauseTitle);
	const auto& pauseToGameTexture = GetGameTextureOrWhite(resources, CommentBoutResourceKey::kPauseToGame);
	const auto& restartTexture = GetGameTextureOrWhite(resources, CommentBoutResourceKey::kRestart);
	const auto& optionTexture = GetGameTextureOrWhite(resources, CommentBoutResourceKey::kOptionMenu);
	const auto& pauseToTitleTexture = GetGameTextureOrWhite(resources, CommentBoutResourceKey::kPauseToTitle);

	auto pauseDimEntity_ = registry.GenerateEntity();
	registry.AddComponent<CBPauseDimTag>(pauseDimEntity_);
	auto* pauseDimTransform = registry.AddComponent<No::Transform2DComponent>(pauseDimEntity_);
	pauseDimTransform->translate = { 640.0f, 360.0f };
	pauseDimTransform->scale = { 1280.0f, 720.0f };
	auto* pauseDimSprite = registry.AddComponent<No::SpriteComponent>(pauseDimEntity_);
	pauseDimSprite->textureHandle = whiteTexture;
	pauseDimSprite->color = { 0.0f, 0.0f, 0.0f, 0.0f };
	pauseDimSprite->layer = CommentBout::ToLayer(CommentBout::SpriteLayer::PauseDim);
	pauseDimSprite->orderInLayer = 0;
	pauseDimSprite->isVisible = false;
	registry.AddComponent<No::EditTag>(pauseDimEntity_)->name = "PauseDim";


	auto pauseMenuBgEntity_ = registry.GenerateEntity();
	auto* bgTr = registry.AddComponent<No::Transform2DComponent>(pauseMenuBgEntity_);
	bgTr->translate = { 640.0f, 430.0f };
	bgTr->scale = { 980.0f, 540.0f };
	auto* bgSp = registry.AddComponent<No::SpriteComponent>(pauseMenuBgEntity_);
	bgSp->textureHandle = whiteTexture;
	bgSp->isVisible = false;
	bgSp->color = { 0.08f, 0.08f, 0.12f, 0.0f };
	bgSp->layer = CommentBout::ToLayer(CommentBout::SpriteLayer::PauseMenuBackground);
	registry.AddComponent<No::EditTag>(pauseMenuBgEntity_)->name = "PauseMenuBackgroundSprite";

	auto pausePanelLineEntity_ = registry.GenerateEntity();
	auto* lineTr = registry.AddComponent<No::Transform2DComponent>(pausePanelLineEntity_);
	lineTr->translate = { 640.0f, 330.0f };
	lineTr->scale = { 980.0f, 8.0f };
	auto* lineSp = registry.AddComponent<No::SpriteComponent>(pausePanelLineEntity_);
	lineSp->textureHandle = whiteTexture;
	lineSp->isVisible = false;
	lineSp->color = { 1.0f, 1.0f, 1.0f, 0.0f };
	lineSp->layer = 0;
	registry.AddComponent<No::EditTag>(pausePanelLineEntity_)->name = "PausePanelLineSprite";

	auto pauseTitleEntity_ = registry.GenerateEntity();
	auto* titleTr = registry.AddComponent<No::Transform2DComponent>(pauseTitleEntity_);
	titleTr->translate = { 640.0f, 220.0f };
	titleTr->scale = { 480.0f, 120.0f };
	auto* titleSp = registry.AddComponent<No::SpriteComponent>(pauseTitleEntity_);
	titleSp->textureHandle = pauseTitleTexture;
	titleSp->isVisible = false;
	titleSp->color = { 1.f, 1.f, 1.f, 0.0f };
	titleSp->layer = CommentBout::ToLayer(CommentBout::SpriteLayer::PauseTitle);
	registry.AddComponent<No::EditTag>(pauseTitleEntity_)->name = "PauseTitleSprite";

	const std::array<NoEngine::TextureRef, 4> itemTextures = {
		pauseToGameTexture, restartTexture, optionTexture, pauseToTitleTexture
	};
	std::array<No::Entity, 4> pauseItemEntities_{};
	for (size_t i = 0; i < pauseItemEntities_.size(); ++i) {
		pauseItemEntities_[i] = registry.GenerateEntity();
		auto* itemTr = registry.AddComponent<No::Transform2DComponent>(pauseItemEntities_[i]);
		itemTr->translate = { 640.0f, 360.0f + 80.0f * static_cast<float>(i) };
		itemTr->scale = { 420.0f, 84.0f };
		auto* itemSp = registry.AddComponent<No::SpriteComponent>(pauseItemEntities_[i]);
		itemSp->textureHandle = itemTextures[i];
		itemSp->isVisible = false;
		itemSp->color = { 1.f, 1.f, 1.f, 0.0f };
		itemSp->layer = CommentBout::ToLayer(CommentBout::SpriteLayer::PauseItem);
		itemSp->orderInLayer = static_cast<uint32_t>(i);
		registry.AddComponent<No::EditTag>(pauseItemEntities_[i])->name = "PauseItemSprite_" + std::to_string(i);
	}

	auto pauseCursorEntity_ = registry.GenerateEntity();
	auto* cursorTr = registry.AddComponent<No::Transform2DComponent>(pauseCursorEntity_);
	cursorTr->translate = { 400.0f, 360.0f };
	cursorTr->scale = { 16.0f, 52.0f };
	auto* cursorSp = registry.AddComponent<No::SpriteComponent>(pauseCursorEntity_);
	cursorSp->textureHandle = whiteTexture;
	cursorSp->isVisible = false;
	cursorSp->color = { 1.0f, 0.95f, 0.35f, 0.0f };
	cursorSp->layer = CommentBout::ToLayer(CommentBout::SpriteLayer::PauseCursor);
	registry.AddComponent<No::EditTag>(pauseCursorEntity_)->name = "PauseCursorSprite";

	pauseView->dimEntity = pauseDimEntity_;
	pauseView->menuBgEntity = pauseMenuBgEntity_;
	pauseView->panelLineEntity = pausePanelLineEntity_;
	pauseView->titleEntity = pauseTitleEntity_;
	pauseView->itemEntities = pauseItemEntities_;
	pauseView->cursorEntity = pauseCursorEntity_;
}
