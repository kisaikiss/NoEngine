#include "stdafx.h"
#include "GameScene.h"
#include <vector>
#include <utility>
#include <array>
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Component/PlayerComponent.h"
#include "application/CommentBout/Component/PlayerAttackComponent.h"
#include "application/CommentBout/Component/PauseStateComponent.h"
#include "application/CommentBout/Component/PauseMenuConfigComponent.h"
#include "application/CommentBout/Component/PauseMenuViewComponent.h"
#include "application/CommentBout/Component/OptionStateComponent.h"
#include "application/CommentBout/Component/OptionMenuConfigComponent.h"
#include "application/CommentBout/Component/OptionMenuViewComponent.h"
#include "application/CommentBout/Component/GrassReactionComponent.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/Component/GroundComponent.h"
#include "application/CommentBout/Utility/CBCollisionMask.h"
#include "application/CommentBout/Utility/CBSpriteLayer.h"
#include "application/CommentBout/Utility/CBGameAudio.h"
#include "application/CommentBout/System/PlayerControlSystem.h"
#include "application/CommentBout/System/GrassReactionSystem.h"
#include "application/CommentBout/System/HitBalloonSystem.h"
#include "application/CommentBout/System/LifetimeSystem.h"
#include "application/CommentBout/System/PauseSystem.h"
#include "application/CommentBout/System/PauseViewSystem.h"
#include "application/CommentBout/System/OptionSystem.h"
#include "application/CommentBout/System/OptionViewSystem.h"
#include "application/TestApp/System/CollisionTestSystem.h"
#include "application/TestApp/Component/Collider2DComponent.h"
#include "application/TestApp/Component/Collider3DComponent.h"
#include "application/TestApp/Component/ProjectedColliderComponent.h"
#include "engine/Runtime/GraphicsCore.h"

void GameScene::Setup() {

	grassNameIndex_ = 0;
	pauseDimEntity_ = No::nullEntity;
	pauseMenuBgEntity_ = No::nullEntity;
	pausePanelLineEntity_ = No::nullEntity;
	pauseTitleEntity_ = No::nullEntity;
	pauseCursorEntity_ = No::nullEntity;
	pauseItemEntities_.fill(No::nullEntity);
	optionDimEntity_ = No::nullEntity;
	optionBgEntity_ = No::nullEntity;
	optionLineEntity_ = No::nullEntity;
	optionTitleEntity_ = No::nullEntity;
	optionItemEntities_.fill(No::nullEntity);
	optionLabelEntities_.fill(No::nullEntity);
	optionBarBaseEntities_.fill(No::nullEntity);
	optionBarFillEntities_.fill(No::nullEntity);
	optionToggleEntity_ = No::nullEntity;
	optionToggleOnEntity_ = No::nullEntity;
	optionToggleOffEntity_ = No::nullEntity;
	optionCursorEntity_ = No::nullEntity;
	// ---- システム登録（順序が重要）----------------------------------------------
	// 1. PlayerControlSystem    : 入力処理・攻撃エンティティのスポawn
	// 2. CollisionTestSystem    : コライダー更新・投影・衝突判定
	//                             → projected->isColliding をセット
	// 3. GrassReactionSystem    : 衝突開始を検出しエフェクトエンティティをスポawn
	//                             → HitBalloonComponent を付与
	// 4. HitBalloonSystem       : HitBalloonComponent が指す投影位置から
	//                             エフェクトの Transform2D を毎フレーム更新
	// 5. LifetimeSystem         : 時間切れエンティティを削除	// ---------------------------------------------------------------------------
	AddSystem(std::make_unique<PauseSystem>());
	AddSystem(std::make_unique<PlayerControlSystem>());
	AddSystem(std::make_unique<TestApp::CollisionTestSystem>());
	AddSystem(std::make_unique<GrassReactionSystem>());
	AddSystem(std::make_unique<HitBalloonSystem>());
	AddSystem(std::make_unique<LifetimeSystem>());
	AddSystem(std::make_unique<OptionSystem>());
	AddSystem(std::make_unique<PauseViewSystem>());
	AddSystem(std::make_unique<OptionViewSystem>());
	AddSystem(std::make_unique<No::DebugCameraSystem>());
	AddSystem(std::make_unique<No::CameraSystem>());
	AddSystem(std::make_unique<No::EditSystem>());

	No::Registry& registry = *GetRegistry();
	CommentBout::GameAudio::InitializeForCommentBout();
	CommentBout::GameAudio::StopTestBGM();
	CommentBout::GameAudio::PlayTestBGM(true);

	const auto whiteTexture = NoEngine::TextureManager::LoadCovertTexture("resources/engine/white1x1.png");

	// 共有リソース
	auto gameResourceEntity = registry.GenerateEntity();
	registry.AddComponent<CBGameResourceTag>(gameResourceEntity);
	auto* gameResource = registry.AddComponent<GameResourceComponent>(gameResourceEntity);
	gameResource->whiteTexture = whiteTexture;

	auto pauseStateEntity = registry.GenerateEntity();
	registry.AddComponent<CBPauseStateTag>(pauseStateEntity);
	auto* pauseState = registry.AddComponent<PauseStateComponent>(pauseStateEntity);
	pauseState->isPaused = false;
	pauseState->selectedIndex = 0;
	pauseState->phase = PauseStateComponent::Closed;
	pauseState->phaseTime = 0.0f;
	pauseState->phaseDuration = 1.0f;
	pauseState->requestedAction = PauseStateComponent::None;
	pauseState->isConfirmAnimating = false;
	pauseState->confirmIndex = -1;
	pauseState->confirmAnimTime = 0.0f;
	auto* pauseStateTag = registry.AddComponent<No::EditTag>(pauseStateEntity);
	pauseStateTag->name = "PauseState";

	auto pauseConfigEntity = registry.GenerateEntity();
	registry.AddComponent<CBPauseConfigTag>(pauseConfigEntity);
	auto* pauseConfig = registry.AddComponent<PauseMenuConfigComponent>(pauseConfigEntity);
	pauseConfig->dimLayer = static_cast<int>(CommentBout::ToLayer(CommentBout::SpriteLayer::PauseDim));
	pauseConfig->menuBgLayer = static_cast<int>(CommentBout::ToLayer(CommentBout::SpriteLayer::PauseMenuBackground));
	pauseConfig->panelLineLayer = static_cast<int>(CommentBout::ToLayer(CommentBout::SpriteLayer::PausePanelLine));
	pauseConfig->titleLayer = static_cast<int>(CommentBout::ToLayer(CommentBout::SpriteLayer::PauseTitle));
	pauseConfig->itemLayer = static_cast<int>(CommentBout::ToLayer(CommentBout::SpriteLayer::PauseItem));
	pauseConfig->cursorLayer = static_cast<int>(CommentBout::ToLayer(CommentBout::SpriteLayer::PauseCursor));
	auto* pauseConfigTag = registry.AddComponent<No::EditTag>(pauseConfigEntity);
	pauseConfigTag->name = "PauseMenuConfig";

	auto optionStateEntity = registry.GenerateEntity();
	registry.AddComponent<CBOptionStateTag>(optionStateEntity);
	auto* optionState = registry.AddComponent<OptionStateComponent>(optionStateEntity);
	optionState->isOpen = false;
	optionState->phase = OptionStateComponent::Closed;
	optionState->phaseDuration = 1.0f;
	optionState->selectedIndex = 0;
	optionState->isEditing = false;
	optionState->isConfirmAnimating = false;
	optionState->confirmIndex = -1;
	optionState->confirmAnimTime = 0.0f;
	optionState->requestedAction = OptionStateComponent::None;
	auto* optionStateTag = registry.AddComponent<No::EditTag>(optionStateEntity);
	optionStateTag->name = "OptionState";

	auto optionConfigEntity = registry.GenerateEntity();
	registry.AddComponent<CBOptionConfigTag>(optionConfigEntity);
	auto* optionConfig = registry.AddComponent<OptionMenuConfigComponent>(optionConfigEntity);
	static_cast<void>(optionConfig);
	auto* optionConfigTag = registry.AddComponent<No::EditTag>(optionConfigEntity);
	optionConfigTag->name = "OptionMenuConfig";

	pauseDimEntity_ = registry.GenerateEntity();
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
	auto* pauseDimTag = registry.AddComponent<No::EditTag>(pauseDimEntity_);
	pauseDimTag->name = "PauseDim";

	CreatePauseMenuSprites(whiteTexture);
	CreateOptionSprites(whiteTexture);

	// ライト
	auto light = registry.GenerateEntity();
	auto* dir = registry.AddComponent<No::DirectionalLightComponent>(light);
	dir->color = { 1.f, 1.f, 1.f, 1.f };
	dir->direction = { 0.f, -1.f, 0.f };
	dir->intensity = 1.f;

	// カメラ
	auto camera = registry.GenerateEntity();
	registry.AddComponent<No::ActiveCameraTag>(camera);
	registry.AddComponent<No::CameraComponent>(camera);
	registry.AddComponent<No::DebugCameraComponent>(camera);
	auto* cameraTag = registry.AddComponent<No::EditTag>(camera);
	cameraTag->name = "camera";
	auto* cameraTransform = registry.AddComponent<No::TransformComponent>(camera);
	cameraTransform->translate.z = -5.f;
	activeCameraEntity_ = camera;

	// 自機スプライト
	auto playerEntity = registry.GenerateEntity();
	registry.AddComponent<CBPlayerTag>(playerEntity);
	auto* imguiName = registry.AddComponent<No::EditTag>(playerEntity);
	imguiName->name = "Player";
	auto* playerComp = registry.AddComponent<PlayerComponent>(playerEntity);
	playerComp->moveSpeed = 480.0f;
	auto* playerAttack = registry.AddComponent<PlayerAttackComponent>(playerEntity);
	playerAttack->spawnOffset = { 0.0f, -80.0f };
	playerAttack->attackSize = { 140.0f, 140.0f };
	playerAttack->visibleTime = 0.35f;
	playerAttack->attackLayer = 30;
	auto* playerTransform = registry.AddComponent<No::Transform2DComponent>(playerEntity);
	playerTransform->translate = { 640.f, 600.f };
	playerTransform->scale = { 128.f, 200.f };
	auto* playerSprite = registry.AddComponent<No::SpriteComponent>(playerEntity);
	playerSprite->layer = CommentBout::ToLayer(CommentBout::SpriteLayer::Gameplay);
	playerSprite->color = { 1.f, 1.f, 1.f, 1.f };
	playerSprite->textureHandle = whiteTexture;
	auto* playerCollider = registry.AddComponent<TestApp::Collider2DComponent>(playerEntity);
	playerCollider->useTransformAsSize = true;
	playerCollider->sizeMultiplier = { 1.f, 1.f };
	playerCollider->collisionLayer = CommentBout::CollisionLayer::CBPlayer;
	playerCollider->collisionMask = CommentBout::CollisionMask::CBPlayer;

	const std::vector<std::pair<No::Vector3, No::Vector3>> grassSpawnParams = {
		{{0.f, 0.f, 6.f}, {2.f, 1.5f, 2.f}},
		{{-2.8f, 0.f, 8.f}, {1.7f, 1.3f, 1.7f}},
		{{2.6f, 0.f, 10.f}, {2.3f, 1.7f, 2.3f}}
	};
	for (const auto& spawnParam : grassSpawnParams) {
		SpawnGrass(spawnParam.first, spawnParam.second);
	}

	// 地面（3D AABB, 現在は何とも当たらない）
	auto groundEntity = registry.GenerateEntity();
	registry.AddComponent<CBGroundTag>(groundEntity);
	registry.AddComponent<GroundComponent>(groundEntity);
	auto* groundTransform = registry.AddComponent<No::TransformComponent>(groundEntity);
	groundTransform->translate = { 0.f, -5.f, 0.f };
	groundTransform->scale = { 10.f, 1.f, 300.f };
	auto* groundTag = registry.AddComponent<No::EditTag>(groundEntity);
	groundTag->name = "Ground";

	auto* groundMesh = registry.AddComponent<No::MeshComponent>(groundEntity);
	auto* groundMaterial = registry.AddComponent<No::MaterialComponent>(groundEntity);
	NoEngine::Asset::ModelLoader::LoadModel(
		"commentbout_ground_cube",
		"resources/game/td_3105/Model/cube/cube.obj",
		groundMesh
	);
	groundMaterial->materials = NoEngine::Asset::ModelLoader::GetMaterial("commentbout_ground_cube");
	groundMaterial->color = { 0.35f, 0.35f, 0.35f, 1.f };
	groundMaterial->psoName = L"Renderer : Default PSO";
	groundMaterial->psoId = NoEngine::Render::GetPSOID(groundMaterial->psoName);
	groundMaterial->rootSigId = NoEngine::Render::GetRootSignatureID(groundMaterial->psoName);

	// auto* groundCollider = registry.AddComponent<TestApp::Collider3DComponent>(groundEntity);
	// groundCollider->shapeType = TestApp::ShapeType3D::Box;
	// groundCollider->useScaleAsBox = true;
	// groundCollider->boxSizeMultiplier = { 1.f, 1.f, 1.f };
	// groundCollider->collisionLayer = CommentBout::CollisionLayer::CBGround;
	// groundCollider->collisionMask = CommentBout::CollisionMask::CBGround;

}

void GameScene::SpawnGrass(const No::Vector3& position, const No::Vector3& size)
{
	No::Registry& registry = *GetRegistry();

	// 草（3D AABB）
	auto grassEntity = registry.GenerateEntity();
	registry.AddComponent<CBGrassTag>(grassEntity);

	registry.AddComponent<GrassReactionComponent>(grassEntity);

	auto* grassTransform = registry.AddComponent<No::TransformComponent>(grassEntity);
	grassTransform->translate = position;
	grassTransform->scale = size;
	auto* grassTag = registry.AddComponent<No::EditTag>(grassEntity);
	grassTag->name = "Grass_" + std::to_string(grassNameIndex_++);

	auto* grassMesh = registry.AddComponent<No::MeshComponent>(grassEntity);
	auto* grassMaterial = registry.AddComponent<No::MaterialComponent>(grassEntity);
	NoEngine::Asset::ModelLoader::LoadModel(
		"commentbout_grass_cube",
		"resources/game/td_3105/Model/cube/cube.obj",
		grassMesh
	);
	grassMaterial->materials = NoEngine::Asset::ModelLoader::GetMaterial("commentbout_grass_cube");
	grassMaterial->color = { 0.2f, 0.8f, 0.2f, 1.f };
	grassMaterial->psoName = L"Renderer : Default PSO";
	grassMaterial->psoId = NoEngine::Render::GetPSOID(grassMaterial->psoName);
	grassMaterial->rootSigId = NoEngine::Render::GetRootSignatureID(grassMaterial->psoName);

	auto* grassCollider = registry.AddComponent<TestApp::Collider3DComponent>(grassEntity);
	grassCollider->shapeType = TestApp::ShapeType3D::Box;
	grassCollider->useScaleAsBox = true;
	grassCollider->boxSizeMultiplier = { 1.f, 1.f, 1.f };
	grassCollider->collisionLayer = CommentBout::CollisionLayer::CBGrass;
	grassCollider->collisionMask = CommentBout::CollisionMask::CBGrass;

	auto* projected = registry.AddComponent<TestApp::ProjectedColliderComponent>(grassEntity);
	projected->source3DEntity = grassEntity;
}

void GameScene::NotSystemUpdate() {
	CameraImGui();
#ifdef USE_IMGUI
	ImGui::Begin("ChangeScene");
	if (ImGui::Button("SceneChangeTitle")) {
		No::SceneChangeEvent event;
		event.nextScene = "TitleScene";
		GetRegistry()->EmitEvent(event);
	}

	if (ImGui::Button("SceneChangeNow")) {
		No::SceneChangeEvent event;
		event.nextScene = "GameScene";
		GetRegistry()->EmitEvent(event);
	}
	ImGui::End();
#endif

}
void GameScene::CameraImGui()
{
#ifdef USE_IMGUI
	ImGui::Begin("CameraControl");
	//camera切り替え
	if (ImGui::Button("CameraChange")) {
		GetRegistry()->AddComponent<No::ActiveCameraTag>(activeCameraEntity_);
	}
	ImGui::End();
#endif // USE_IMGUI
}

void GameScene::CreatePauseMenuSprites(const NoEngine::TextureRef& whiteTexture)
{
	No::Registry& registry = *GetRegistry();
	auto pauseViewEntity = registry.GenerateEntity();
	registry.AddComponent<CBPauseViewTag>(pauseViewEntity);
	auto* pauseView = registry.AddComponent<PauseMenuViewComponent>(pauseViewEntity);
	const auto pauseTitleTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/Pause.png");
	const auto pauseToGameTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/PauseToGame.png");
	const auto restartTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/Restart.png");
	const auto optionTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/OptionMenu.png");
	const auto pauseToTitleTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/PauseToTitle.png");

	pauseMenuBgEntity_ = registry.GenerateEntity();
	auto* bgTransform = registry.AddComponent<No::Transform2DComponent>(pauseMenuBgEntity_);
	bgTransform->translate = { 640.0f, 430.0f };
	bgTransform->scale = { 980.0f, 540.0f };
	auto* bgSprite = registry.AddComponent<No::SpriteComponent>(pauseMenuBgEntity_);
	bgSprite->textureHandle = whiteTexture;
	bgSprite->isVisible = false;
	bgSprite->color = { 0.08f, 0.08f, 0.12f, 0.0f };
	bgSprite->layer = CommentBout::ToLayer(CommentBout::SpriteLayer::PauseMenuBackground);
	auto* bgTag = registry.AddComponent<No::EditTag>(pauseMenuBgEntity_);
	bgTag->name = "PauseMenuBackgroundSprite";

	pausePanelLineEntity_ = registry.GenerateEntity();
	auto* lineTransform = registry.AddComponent<No::Transform2DComponent>(pausePanelLineEntity_);
	lineTransform->translate = { 640.0f, 330.0f };
	lineTransform->scale = { 980.0f, 8.0f };
	auto* lineSprite = registry.AddComponent<No::SpriteComponent>(pausePanelLineEntity_);
	lineSprite->textureHandle = whiteTexture;
	lineSprite->isVisible = false;
	lineSprite->color = { 1.0f, 1.0f, 1.0f, 0.0f };
	lineSprite->layer = 0;
	auto* lineTag = registry.AddComponent<No::EditTag>(pausePanelLineEntity_);
	lineTag->name = "PausePanelLineSprite";

	pauseTitleEntity_ = registry.GenerateEntity();
	auto* titleTransform = registry.AddComponent<No::Transform2DComponent>(pauseTitleEntity_);
	titleTransform->translate = { 640.0f, 220.0f };
	titleTransform->scale = { 480.0f, 120.0f };
	auto* titleSprite = registry.AddComponent<No::SpriteComponent>(pauseTitleEntity_);
	titleSprite->textureHandle = pauseTitleTexture;
	titleSprite->isVisible = false;
	titleSprite->color = { 1.f, 1.f, 1.f, 0.0f };
	titleSprite->layer = CommentBout::ToLayer(CommentBout::SpriteLayer::PauseTitle);
	auto* titleTag = registry.AddComponent<No::EditTag>(pauseTitleEntity_);
	titleTag->name = "PauseTitleSprite";

	const std::array<NoEngine::TextureRef, 4> itemTextures = {
		pauseToGameTexture,
		restartTexture,
		optionTexture,
		pauseToTitleTexture
	};

	for (size_t i = 0; i < pauseItemEntities_.size(); ++i) {
		pauseItemEntities_[i] = registry.GenerateEntity();
		auto* itemTransform = registry.AddComponent<No::Transform2DComponent>(pauseItemEntities_[i]);
		itemTransform->translate = { 640.0f, 360.0f + 80.0f * static_cast<float>(i) };
		itemTransform->scale = { 420.0f, 84.0f };
		auto* itemSprite = registry.AddComponent<No::SpriteComponent>(pauseItemEntities_[i]);
		itemSprite->textureHandle = itemTextures[i];
		itemSprite->isVisible = false;
		itemSprite->color = { 1.f, 1.f, 1.f, 0.0f };
		itemSprite->layer = CommentBout::ToLayer(CommentBout::SpriteLayer::PauseItem);
		itemSprite->orderInLayer = static_cast<uint32_t>(i);
		auto* itemTag = registry.AddComponent<No::EditTag>(pauseItemEntities_[i]);
		itemTag->name = "PauseItemSprite_" + std::to_string(i);
	}

	pauseCursorEntity_ = registry.GenerateEntity();
	auto* cursorTransform = registry.AddComponent<No::Transform2DComponent>(pauseCursorEntity_);
	cursorTransform->translate = { 400.0f, 360.0f };
	cursorTransform->scale = { 16.0f, 52.0f };
	auto* cursorSprite = registry.AddComponent<No::SpriteComponent>(pauseCursorEntity_);
	cursorSprite->textureHandle = whiteTexture;
	cursorSprite->isVisible = false;
	cursorSprite->color = { 1.0f, 0.95f, 0.35f, 0.0f };
	cursorSprite->layer = CommentBout::ToLayer(CommentBout::SpriteLayer::PauseCursor);
	auto* cursorTag = registry.AddComponent<No::EditTag>(pauseCursorEntity_);
	cursorTag->name = "PauseCursorSprite";

	pauseView->dimEntity = pauseDimEntity_;
	pauseView->menuBgEntity = pauseMenuBgEntity_;
	pauseView->panelLineEntity = pausePanelLineEntity_;
	pauseView->titleEntity = pauseTitleEntity_;
	pauseView->itemEntities = pauseItemEntities_;
	pauseView->cursorEntity = pauseCursorEntity_;
}

void GameScene::CreateOptionSprites(const NoEngine::TextureRef& whiteTexture)
{
	No::Registry& registry = *GetRegistry();
	auto optionViewEntity = registry.GenerateEntity();
	registry.AddComponent<CBOptionViewTag>(optionViewEntity);
	auto* optionView = registry.AddComponent<OptionMenuViewComponent>(optionViewEntity);

	const auto optionTitleTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/OptionMenu.png");
	const auto masterTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/Master.png");
	const auto bgmTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/BGM.png");
	const auto seTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/SE.png");
	const auto vibrationTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/Vibration.png");
	const auto backTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/Back.png");
	const auto onTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/On.png");
	const auto offTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/Off.png");

	optionDimEntity_ = registry.GenerateEntity();
	auto* dimTransform = registry.AddComponent<No::Transform2DComponent>(optionDimEntity_);
	dimTransform->translate = { 640.0f, 360.0f };
	dimTransform->scale = { 1280.0f, 720.0f };
	auto* dimSprite = registry.AddComponent<No::SpriteComponent>(optionDimEntity_);
	dimSprite->textureHandle = whiteTexture;
	dimSprite->isVisible = false;
	dimSprite->color = { 0.0f, 0.0f, 0.0f, 0.0f };
	auto* dimTag = registry.AddComponent<No::EditTag>(optionDimEntity_);
	dimTag->name = "OptionDim";

	optionBgEntity_ = registry.GenerateEntity();
	auto* bgTransform = registry.AddComponent<No::Transform2DComponent>(optionBgEntity_);
	bgTransform->translate = { 640.0f, 400.0f };
	bgTransform->scale = { 960.0f, 520.0f };
	auto* bgSprite = registry.AddComponent<No::SpriteComponent>(optionBgEntity_);
	bgSprite->textureHandle = whiteTexture;
	bgSprite->isVisible = false;
	bgSprite->color = { 0.06f, 0.06f, 0.08f, 0.0f };
	auto* bgTag = registry.AddComponent<No::EditTag>(optionBgEntity_);
	bgTag->name = "OptionBackground";

	optionLineEntity_ = registry.GenerateEntity();
	auto* lineTransform = registry.AddComponent<No::Transform2DComponent>(optionLineEntity_);
	lineTransform->translate = { 640.0f, 220.0f };
	lineTransform->scale = { 920.0f, 6.0f };
	auto* lineSprite = registry.AddComponent<No::SpriteComponent>(optionLineEntity_);
	lineSprite->textureHandle = whiteTexture;
	lineSprite->isVisible = false;
	lineSprite->color = { 1.0f, 1.0f, 1.0f, 0.0f };
	lineSprite->layer = 0;
	auto* lineTag = registry.AddComponent<No::EditTag>(optionLineEntity_);
	lineTag->name = "OptionLine";

	optionTitleEntity_ = registry.GenerateEntity();
	auto* titleTransform = registry.AddComponent<No::Transform2DComponent>(optionTitleEntity_);
	titleTransform->translate = { 920.0f, 120.0f };
	titleTransform->scale = { 280.0f, 58.0f };
	auto* titleSprite = registry.AddComponent<No::SpriteComponent>(optionTitleEntity_);
	titleSprite->textureHandle = optionTitleTexture;
	titleSprite->isVisible = false;
	titleSprite->color = { 1.0f, 1.0f, 1.0f, 0.0f };
	titleSprite->layer = 0;
	auto* titleTag = registry.AddComponent<No::EditTag>(optionTitleEntity_);
	titleTag->name = "OptionTitle";

	for (size_t i = 0; i < optionItemEntities_.size(); ++i) {
		optionItemEntities_[i] = registry.GenerateEntity();
		auto* itemTransform = registry.AddComponent<No::Transform2DComponent>(optionItemEntities_[i]);
		itemTransform->translate = { 640.0f, 300.0f + static_cast<float>(i) * 72.0f };
		itemTransform->scale = { 760.0f, 60.0f };
		auto* itemSprite = registry.AddComponent<No::SpriteComponent>(optionItemEntities_[i]);
		itemSprite->textureHandle = whiteTexture;
		itemSprite->isVisible = false;
		itemSprite->color = { 0.20f, 0.20f, 0.24f, 0.0f };
		auto* itemTag = registry.AddComponent<No::EditTag>(optionItemEntities_[i]);
		itemTag->name = "OptionItem_" + std::to_string(i);
	}

	const std::array<NoEngine::TextureRef, 5> labelTextures = {
		masterTexture,
		bgmTexture,
		seTexture,
		vibrationTexture,
		backTexture
	};
	for (size_t i = 0; i < optionLabelEntities_.size(); ++i) {
		optionLabelEntities_[i] = registry.GenerateEntity();
		auto* tr = registry.AddComponent<No::Transform2DComponent>(optionLabelEntities_[i]);
		tr->translate = { 760.0f, 300.0f + static_cast<float>(i) * 72.0f };
		tr->scale = { 250.0f, 42.0f };
		auto* sp = registry.AddComponent<No::SpriteComponent>(optionLabelEntities_[i]);
		sp->textureHandle = labelTextures[i];
		sp->isVisible = false;
		sp->color = { 1.0f, 1.0f, 1.0f, 0.0f };
		auto* tag = registry.AddComponent<No::EditTag>(optionLabelEntities_[i]);
		tag->name = "OptionLabel_" + std::to_string(i);
	}

	for (size_t i = 0; i < optionBarBaseEntities_.size(); ++i) {
		optionBarBaseEntities_[i] = registry.GenerateEntity();
		auto* barBaseTransform = registry.AddComponent<No::Transform2DComponent>(optionBarBaseEntities_[i]);
		barBaseTransform->translate = { 800.0f, 300.0f + static_cast<float>(i) * 72.0f };
		barBaseTransform->scale = { 360.0f, 16.0f };
		auto* barBaseSprite = registry.AddComponent<No::SpriteComponent>(optionBarBaseEntities_[i]);
		barBaseSprite->textureHandle = whiteTexture;
		barBaseSprite->isVisible = false;
		barBaseSprite->color = { 0.10f, 0.10f, 0.10f, 0.0f };

		optionBarFillEntities_[i] = registry.GenerateEntity();
		auto* barFillTransform = registry.AddComponent<No::Transform2DComponent>(optionBarFillEntities_[i]);
		barFillTransform->translate = { 800.0f, 300.0f + static_cast<float>(i) * 72.0f };
		barFillTransform->scale = { 180.0f, 16.0f };
		auto* barFillSprite = registry.AddComponent<No::SpriteComponent>(optionBarFillEntities_[i]);
		barFillSprite->textureHandle = whiteTexture;
		barFillSprite->isVisible = false;
		barFillSprite->color = { 0.95f, 0.90f, 0.28f, 0.0f };
	}

	optionToggleEntity_ = registry.GenerateEntity();
	auto* toggleTransform = registry.AddComponent<No::Transform2DComponent>(optionToggleEntity_);
	toggleTransform->translate = { 810.0f, 300.0f + 3.0f * 72.0f };
	toggleTransform->scale = { 154.7f, 34.8f };
	auto* toggleSprite = registry.AddComponent<No::SpriteComponent>(optionToggleEntity_);
	toggleSprite->textureHandle = whiteTexture;
	toggleSprite->isVisible = false;
	toggleSprite->color = { 0.20f, 0.75f, 0.30f, 0.0f };
	auto* toggleTag = registry.AddComponent<No::EditTag>(optionToggleEntity_);
	toggleTag->name = "OptionToggleBg";

	optionToggleOnEntity_ = registry.GenerateEntity();
	auto* onTransform = registry.AddComponent<No::Transform2DComponent>(optionToggleOnEntity_);
	onTransform->translate = { 810.0f, 300.0f + 3.0f * 72.0f };
	onTransform->scale = { 154.7f, 34.8f };
	auto* onSprite = registry.AddComponent<No::SpriteComponent>(optionToggleOnEntity_);
	onSprite->textureHandle = onTexture;
	onSprite->isVisible = false;
	onSprite->color = { 1.0f, 1.0f, 1.0f, 0.0f };

	optionToggleOffEntity_ = registry.GenerateEntity();
	auto* offTransform = registry.AddComponent<No::Transform2DComponent>(optionToggleOffEntity_);
	offTransform->translate = { 810.0f, 300.0f + 3.0f * 72.0f };
	offTransform->scale = { 154.7f, 34.8f };
	auto* offSprite = registry.AddComponent<No::SpriteComponent>(optionToggleOffEntity_);
	offSprite->textureHandle = offTexture;
	offSprite->isVisible = false;
	offSprite->color = { 1.0f, 1.0f, 1.0f, 0.0f };

	optionCursorEntity_ = registry.GenerateEntity();
	auto* cursorTransform = registry.AddComponent<No::Transform2DComponent>(optionCursorEntity_);
	cursorTransform->translate = { 680.0f, 300.0f };
	cursorTransform->scale = { 14.0f, 48.0f };
	auto* cursorSprite = registry.AddComponent<No::SpriteComponent>(optionCursorEntity_);
	cursorSprite->textureHandle = whiteTexture;
	cursorSprite->isVisible = false;
	cursorSprite->color = { 1.0f, 1.0f, 1.0f, 0.0f };
	auto* cursorTag = registry.AddComponent<No::EditTag>(optionCursorEntity_);
	cursorTag->name = "OptionCursor";

	optionView->dimEntity = optionDimEntity_;
	optionView->bgEntity = optionBgEntity_;
	optionView->lineEntity = optionLineEntity_;
	optionView->titleEntity = optionTitleEntity_;
	optionView->itemEntities = optionItemEntities_;
	optionView->labelEntities = optionLabelEntities_;
	optionView->barBaseEntities = optionBarBaseEntities_;
	optionView->barFillEntities = optionBarFillEntities_;
	optionView->toggleEntity = optionToggleEntity_;
	optionView->toggleOnEntity = optionToggleOnEntity_;
	optionView->toggleOffEntity = optionToggleOffEntity_;
	optionView->cursorEntity = optionCursorEntity_;
}
