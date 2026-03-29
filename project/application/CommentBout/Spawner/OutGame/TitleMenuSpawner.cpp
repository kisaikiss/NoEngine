#include "stdafx.h"
#include "TitleMenuSpawner.h"
#include "application/CommentBout/Component/OutGame/TitleMenuViewComponent.h"
#include "application/CommentBout/GameTag.h"

void TitleMenuSpawner::Create(No::Registry& registry, const GameResourceComponent& resources)
{
	const auto& logoTexture = GetGameTextureOrWhite(resources, CommentBoutResourceKey::kTitleLogo);
	const auto& startTexture = GetGameTextureOrWhite(resources, CommentBoutResourceKey::kGameStart);
	const auto& optionTexture = GetGameTextureOrWhite(resources, CommentBoutResourceKey::kOptionMenu);
	const auto& endTexture = GetGameTextureOrWhite(resources, CommentBoutResourceKey::kGameEnd);
	const auto& whiteTexture = GetGameTextureOrWhite(resources, CommentBoutResourceKey::kWhiteTexture);

	auto viewEntity = registry.GenerateEntity();
	registry.AddComponent<CBTitleViewTag>(viewEntity);
	auto* view = registry.AddComponent<TitleMenuViewComponent>(viewEntity);

	const auto backgroundEntity = registry.GenerateEntity();
	auto* bgTr = registry.AddComponent<No::Transform2DComponent>(backgroundEntity);
	bgTr->translate = { 640.0f, 360.0f };
	bgTr->scale = { 1280.0f, 720.0f };
	auto* bgSp = registry.AddComponent<No::SpriteComponent>(backgroundEntity);
	bgSp->textureHandle = whiteTexture;
	bgSp->isVisible = true;
	bgSp->color = { 0.08f, 0.08f, 0.12f, 1.0f };
	registry.AddComponent<No::EditTag>(backgroundEntity)->name = "TitleBackground";

	const auto panelEntity = registry.GenerateEntity();
	auto* panelTr = registry.AddComponent<No::Transform2DComponent>(panelEntity);
	panelTr->translate = { 640.0f, 360.0f };
	panelTr->scale = { 1080.0f, 640.0f };
	auto* panelSp = registry.AddComponent<No::SpriteComponent>(panelEntity);
	panelSp->textureHandle = whiteTexture;
	panelSp->isVisible = true;
	panelSp->color = { 0.95f, 0.55f, 0.12f, 0.92f };
	registry.AddComponent<No::EditTag>(panelEntity)->name = "TitlePanel";

	const auto logoEntity = registry.GenerateEntity();
	auto* logoTr = registry.AddComponent<No::Transform2DComponent>(logoEntity);
	logoTr->translate = { 640.0f, 180.0f };
	logoTr->scale = { 620.0f, 180.0f };
	auto* logoSp = registry.AddComponent<No::SpriteComponent>(logoEntity);
	logoSp->textureHandle = logoTexture;
	logoSp->isVisible = true;
	logoSp->color = No::Color::WHITE;
	registry.AddComponent<No::EditTag>(logoEntity)->name = "TitleLogo";

	const std::array<NoEngine::TextureRef, 3> itemTextures = {
		startTexture,
		optionTexture,
		endTexture
	};
	std::array<No::Entity, 3> itemEntities{};
	for (size_t i = 0; i < itemEntities.size(); ++i) {
		itemEntities[i] = registry.GenerateEntity();
		auto* itemTr = registry.AddComponent<No::Transform2DComponent>(itemEntities[i]);
		itemTr->translate = { 640.0f, 395.0f + 95.0f * static_cast<float>(i) };
		itemTr->scale = { 420.0f, 90.0f };
		auto* itemSp = registry.AddComponent<No::SpriteComponent>(itemEntities[i]);
		itemSp->textureHandle = itemTextures[i];
		itemSp->isVisible = true;
		itemSp->color = { 0.85f, 0.85f, 0.85f, 1.0f };
		registry.AddComponent<No::EditTag>(itemEntities[i])->name = "TitleItem_" + std::to_string(i);
	}

	const auto cursorEntity = registry.GenerateEntity();
	auto* cursorTr = registry.AddComponent<No::Transform2DComponent>(cursorEntity);
	cursorTr->translate = { 340.0f, 395.0f };
	cursorTr->scale = { 28.0f, 60.0f };
	auto* cursorSp = registry.AddComponent<No::SpriteComponent>(cursorEntity);
	cursorSp->textureHandle = whiteTexture;
	cursorSp->isVisible = true;
	cursorSp->color = { 1.0f, 0.95f, 0.35f, 1.0f };
	registry.AddComponent<No::EditTag>(cursorEntity)->name = "TitleCursor";

	view->backgroundEntity = backgroundEntity;
	view->panelEntity = panelEntity;
	view->logoEntity = logoEntity;
	view->itemEntities = itemEntities;
	view->cursorEntity = cursorEntity;
}
