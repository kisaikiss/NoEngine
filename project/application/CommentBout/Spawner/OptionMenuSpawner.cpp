#include "stdafx.h"
#include "OptionMenuSpawner.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Component/OptionMenuViewComponent.h"

void OptionMenuSpawner::Create(No::Registry& registry, const NoEngine::TextureRef& whiteTexture)
{

	const auto optionTitleTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/OptionMenu.png");
	const auto masterTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/Master.png");
	const auto bgmTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/BGM.png");
	const auto seTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/SE.png");
	const auto vibrationTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/Vibration.png");
	const auto backTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/Back.png");
	const auto onTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/On.png");
	const auto offTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/Off.png");


	const auto optionViewEntity = registry.GenerateEntity();
	registry.AddComponent<CBOptionViewTag>(optionViewEntity);
	auto* optionView = registry.AddComponent<OptionMenuViewComponent>(optionViewEntity);


	const auto dimEntity = registry.GenerateEntity();
	auto* dimTr = registry.AddComponent<No::Transform2DComponent>(dimEntity);
	dimTr->translate = { 640.0f, 360.0f };
	dimTr->scale = { 1280.0f, 720.0f };
	auto* dimSp = registry.AddComponent<No::SpriteComponent>(dimEntity);
	dimSp->textureHandle = whiteTexture;
	dimSp->isVisible = false;
	dimSp->color = { 0.0f, 0.0f, 0.0f, 0.0f };
	registry.AddComponent<No::EditTag>(dimEntity)->name = "OptionDim";


	const auto bgEntity = registry.GenerateEntity();
	auto* bgTr = registry.AddComponent<No::Transform2DComponent>(bgEntity);
	bgTr->translate = { 640.0f, 760.0f };
	bgTr->scale = { 960.0f, 520.0f };
	auto* bgSp = registry.AddComponent<No::SpriteComponent>(bgEntity);
	bgSp->textureHandle = whiteTexture;
	bgSp->isVisible = false;
	bgSp->color = { 1.0f, 0.6f, 0.0f, 0.0f };
	registry.AddComponent<No::EditTag>(bgEntity)->name = "OptionBackground";


	const auto lineEntity = registry.GenerateEntity();
	auto* lineTr = registry.AddComponent<No::Transform2DComponent>(lineEntity);
	lineTr->translate = { 857.20f, -130.20f };
	lineTr->scale = { 919.70f, 16.0f };
	auto* lineSp = registry.AddComponent<No::SpriteComponent>(lineEntity);
	lineSp->textureHandle = whiteTexture;
	lineSp->isVisible = false;
	lineSp->color = { 1.0f, 1.0f, 1.0f, 0.0f };
	registry.AddComponent<No::EditTag>(lineEntity)->name = "OptionLine";


	const auto titleEntity = registry.GenerateEntity();
	auto* titleTr = registry.AddComponent<No::Transform2DComponent>(titleEntity);
	titleTr->translate = { 1383.20f, 248.40f };
	titleTr->scale = { 301.80f, 65.50f };
	auto* titleSp = registry.AddComponent<No::SpriteComponent>(titleEntity);
	titleSp->textureHandle = optionTitleTexture;
	titleSp->isVisible = false;
	titleSp->color = { 1.0f, 1.0f, 1.0f, 0.0f };
	registry.AddComponent<No::EditTag>(titleEntity)->name = "OptionTitle";


	const std::array<NoEngine::TextureRef, 5> labelTextures = {
		masterTexture, bgmTexture, seTexture, vibrationTexture, backTexture
	};

	std::array<No::Entity, 5> itemEntities{};
	std::array<No::Entity, 5> labelEntities{};
	std::array<No::Entity, 3> barBaseEntities{};
	std::array<No::Entity, 3> barFillEntities{};
	std::array<No::Entity, 5> overlayEntities{};

	for (size_t i = 0; i < itemEntities.size(); ++i) {
		// アイテム背景
		itemEntities[i] = registry.GenerateEntity();
		auto* itemTr = registry.AddComponent<No::Transform2DComponent>(itemEntities[i]);
		itemTr->translate = { 640.0f, 860.0f + 69.60f * static_cast<float>(i) };
		itemTr->scale = { 620.0f, 52.70f };
		auto* itemSp = registry.AddComponent<No::SpriteComponent>(itemEntities[i]);
		itemSp->textureHandle = whiteTexture;
		itemSp->isVisible = false;
		itemSp->color = { 0.20f, 0.20f, 0.24f, 0.0f };
		registry.AddComponent<No::EditTag>(itemEntities[i])->name = "OptionItem_" + std::to_string(i);

		// ラベル
		labelEntities[i] = registry.GenerateEntity();
		auto* labelTr = registry.AddComponent<No::Transform2DComponent>(labelEntities[i]);
		labelTr->translate = { 640.0f - 191.50f, 860.0f + 69.60f * static_cast<float>(i) };
		labelTr->scale = { 226.50f, 42.0f };
		auto* labelSp = registry.AddComponent<No::SpriteComponent>(labelEntities[i]);
		labelSp->textureHandle = labelTextures[i];
		labelSp->isVisible = false;
		labelSp->color = { 1.0f, 1.0f, 1.0f, 0.0f };
		registry.AddComponent<No::EditTag>(labelEntities[i])->name = "OptionLabel_" + std::to_string(i);

		// 音量バー（index 0〜2 のみ）
		if (i < barBaseEntities.size()) {
			barBaseEntities[i] = registry.GenerateEntity();
			auto* baseTr = registry.AddComponent<No::Transform2DComponent>(barBaseEntities[i]);
			baseTr->translate = { 640.0f + 110.0f, 860.0f + 69.60f * static_cast<float>(i) };
			baseTr->scale = { 360.0f, 16.0f };
			auto* baseSp = registry.AddComponent<No::SpriteComponent>(barBaseEntities[i]);
			baseSp->textureHandle = whiteTexture;
			baseSp->isVisible = false;
			baseSp->color = { 0.10f, 0.10f, 0.10f, 0.0f };

			barFillEntities[i] = registry.GenerateEntity();
			auto* fillTr = registry.AddComponent<No::Transform2DComponent>(barFillEntities[i]);
			fillTr->translate = { 640.0f + 110.0f, 860.0f + 69.60f * static_cast<float>(i) };
			fillTr->scale = { 180.0f, 16.0f };
			auto* fillSp = registry.AddComponent<No::SpriteComponent>(barFillEntities[i]);
			fillSp->textureHandle = whiteTexture;
			fillSp->isVisible = false;
			fillSp->color = { 0.95f, 0.90f, 0.28f, 0.0f };
		}

		// 編集モード用 行暗幕オーバーレイ
		overlayEntities[i] = registry.GenerateEntity();
		auto* ovTr = registry.AddComponent<No::Transform2DComponent>(overlayEntities[i]);
		ovTr->translate = { 640.0f, 860.0f + 69.60f * static_cast<float>(i) };
		ovTr->scale = { 620.0f, 52.70f };
		auto* ovSp = registry.AddComponent<No::SpriteComponent>(overlayEntities[i]);
		ovSp->textureHandle = whiteTexture;
		ovSp->isVisible = false;
		ovSp->color = { 0.0f, 0.0f, 0.0f, 0.0f };
		registry.AddComponent<No::EditTag>(overlayEntities[i])->name = "OptionOverlay_" + std::to_string(i);
	}


	const auto toggleEntity = registry.GenerateEntity();
	auto* toggleTr = registry.AddComponent<No::Transform2DComponent>(toggleEntity);
	toggleTr->translate = { 640.0f + 98.30f, 860.0f + 69.60f * 3.0f };
	toggleTr->scale = { 126.20f, 52.0f };
	auto* toggleSp = registry.AddComponent<No::SpriteComponent>(toggleEntity);
	toggleSp->textureHandle = whiteTexture;
	toggleSp->isVisible = false;
	toggleSp->color = { 0.20f, 0.20f, 0.24f, 0.0f };
	registry.AddComponent<No::EditTag>(toggleEntity)->name = "OptionToggleBg";

	const auto toggleOnEntity = registry.GenerateEntity();
	auto* onTr = registry.AddComponent<No::Transform2DComponent>(toggleOnEntity);
	onTr->translate = { 640.0f + 98.30f, 860.0f + 69.60f * 3.0f };
	onTr->scale = { 126.20f, 52.0f };
	auto* onSp = registry.AddComponent<No::SpriteComponent>(toggleOnEntity);
	onSp->textureHandle = onTexture;
	onSp->isVisible = false;
	onSp->color = { 1.0f, 1.0f, 1.0f, 0.0f };

	const auto toggleOffEntity = registry.GenerateEntity();
	auto* offTr = registry.AddComponent<No::Transform2DComponent>(toggleOffEntity);
	offTr->translate = { 640.0f + 98.30f, 860.0f + 69.60f * 3.0f };
	offTr->scale = { 126.20f, 52.0f };
	auto* offSp = registry.AddComponent<No::SpriteComponent>(toggleOffEntity);
	offSp->textureHandle = offTexture;
	offSp->isVisible = false;
	offSp->color = { 1.0f, 1.0f, 1.0f, 0.0f };

	const auto cursorEntity = registry.GenerateEntity();
	auto* cursorTr = registry.AddComponent<No::Transform2DComponent>(cursorEntity);
	cursorTr->translate = { 640.0f - 276.20f, 860.0f };
	cursorTr->scale = { 28.0f, 28.0f };
	auto* cursorSp = registry.AddComponent<No::SpriteComponent>(cursorEntity);
	cursorSp->textureHandle = whiteTexture;
	cursorSp->isVisible = false;
	cursorSp->color = { 1.0f, 1.0f, 1.0f, 0.0f };
	registry.AddComponent<No::EditTag>(cursorEntity)->name = "OptionCursor";

	// ---- ViewComponent への登録 --------------------------------------------
	optionView->dimEntity = dimEntity;
	optionView->bgEntity = bgEntity;
	optionView->lineEntity = lineEntity;
	optionView->titleEntity = titleEntity;
	optionView->itemEntities = itemEntities;
	optionView->labelEntities = labelEntities;
	optionView->barBaseEntities = barBaseEntities;
	optionView->barFillEntities = barFillEntities;
	optionView->toggleEntity = toggleEntity;
	optionView->toggleOnEntity = toggleOnEntity;
	optionView->toggleOffEntity = toggleOffEntity;
	optionView->cursorEntity = cursorEntity;
	optionView->itemOverlayEntities = overlayEntities;
}