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
#include "application/CommentBout/Spawner/OptionMenuSpawner.h"  // ★追加
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
	registry.AddComponent<No::EditTag>(pauseStateEntity)->name = "PauseState";


	auto pauseConfigEntity = registry.GenerateEntity();
	registry.AddComponent<CBPauseConfigTag>(pauseConfigEntity);
	auto* pauseConfig = registry.AddComponent<PauseMenuConfigComponent>(pauseConfigEntity);
	pauseConfig->dimLayer = static_cast<int>(CommentBout::ToLayer(CommentBout::SpriteLayer::PauseDim));
	pauseConfig->menuBgLayer = static_cast<int>(CommentBout::ToLayer(CommentBout::SpriteLayer::PauseMenuBackground));
	pauseConfig->panelLineLayer = static_cast<int>(CommentBout::ToLayer(CommentBout::SpriteLayer::PausePanelLine));
	pauseConfig->titleLayer = static_cast<int>(CommentBout::ToLayer(CommentBout::SpriteLayer::PauseTitle));
	pauseConfig->itemLayer = static_cast<int>(CommentBout::ToLayer(CommentBout::SpriteLayer::PauseItem));
	pauseConfig->cursorLayer = static_cast<int>(CommentBout::ToLayer(CommentBout::SpriteLayer::PauseCursor));
	registry.AddComponent<No::EditTag>(pauseConfigEntity)->name = "PauseMenuConfig";


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
	registry.AddComponent<No::EditTag>(optionStateEntity)->name = "OptionState";

	
	auto optionConfigEntity = registry.GenerateEntity();
	registry.AddComponent<CBOptionConfigTag>(optionConfigEntity);
	registry.AddComponent<OptionMenuConfigComponent>(optionConfigEntity); // デフォルト値 = JSON値
	registry.AddComponent<No::EditTag>(optionConfigEntity)->name = "OptionMenuConfig";


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
	registry.AddComponent<No::EditTag>(pauseDimEntity_)->name = "PauseDim";

	CreatePauseMenuSprites(whiteTexture);


	OptionMenuSpawner::Create(registry, whiteTexture);

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
	registry.AddComponent<No::EditTag>(camera)->name = "camera";
	auto* cameraTransform = registry.AddComponent<No::TransformComponent>(camera);
	cameraTransform->translate.z = -5.f;
	activeCameraEntity_ = camera;
	// 自機スプライト
	auto playerEntity = registry.GenerateEntity();
	registry.AddComponent<CBPlayerTag>(playerEntity);
	registry.AddComponent<No::EditTag>(playerEntity)->name = "Player";
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
		{{0.f, 0.f, 6.f},   {2.f, 1.5f, 2.f}},
		{{-2.8f, 0.f, 8.f}, {1.7f, 1.3f, 1.7f}},
		{{2.6f, 0.f, 10.f}, {2.3f, 1.7f, 2.3f}}
	};
	for (const auto& p : grassSpawnParams) {
		SpawnGrass(p.first, p.second);
	}

	// 地面（3D AABB, 現在は何とも当たらない）
	auto groundEntity = registry.GenerateEntity();
	registry.AddComponent<CBGroundTag>(groundEntity);
	registry.AddComponent<GroundComponent>(groundEntity);
	auto* groundTransform = registry.AddComponent<No::TransformComponent>(groundEntity);
	groundTransform->translate = { 0.f, -5.f, 0.f };
	groundTransform->scale = { 10.f, 1.f, 300.f };
	registry.AddComponent<No::EditTag>(groundEntity)->name = "Ground";
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
}

void GameScene::SpawnGrass(const No::Vector3& position, const No::Vector3& size)
{
	No::Registry& registry = *GetRegistry();

	auto grassEntity = registry.GenerateEntity();
	registry.AddComponent<CBGrassTag>(grassEntity);
	registry.AddComponent<GrassReactionComponent>(grassEntity);
	auto* grassTransform = registry.AddComponent<No::TransformComponent>(grassEntity);
	grassTransform->translate = position;
	grassTransform->scale = size;
	registry.AddComponent<No::EditTag>(grassEntity)->name = "Grass_" + std::to_string(grassNameIndex_++);
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

void GameScene::NotSystemUpdate()
{
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
	if (ImGui::Button("CameraChange")) {
		GetRegistry()->AddComponent<No::ActiveCameraTag>(activeCameraEntity_);
	}
	ImGui::End();
#endif
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
	auto* bgTr = registry.AddComponent<No::Transform2DComponent>(pauseMenuBgEntity_);
	bgTr->translate = { 640.0f, 430.0f };
	bgTr->scale = { 980.0f, 540.0f };
	auto* bgSp = registry.AddComponent<No::SpriteComponent>(pauseMenuBgEntity_);
	bgSp->textureHandle = whiteTexture;
	bgSp->isVisible = false;
	bgSp->color = { 0.08f, 0.08f, 0.12f, 0.0f };
	bgSp->layer = CommentBout::ToLayer(CommentBout::SpriteLayer::PauseMenuBackground);
	registry.AddComponent<No::EditTag>(pauseMenuBgEntity_)->name = "PauseMenuBackgroundSprite";

	pausePanelLineEntity_ = registry.GenerateEntity();
	auto* lineTr = registry.AddComponent<No::Transform2DComponent>(pausePanelLineEntity_);
	lineTr->translate = { 640.0f, 330.0f };
	lineTr->scale = { 980.0f, 8.0f };
	auto* lineSp = registry.AddComponent<No::SpriteComponent>(pausePanelLineEntity_);
	lineSp->textureHandle = whiteTexture;
	lineSp->isVisible = false;
	lineSp->color = { 1.0f, 1.0f, 1.0f, 0.0f };
	lineSp->layer = 0;
	registry.AddComponent<No::EditTag>(pausePanelLineEntity_)->name = "PausePanelLineSprite";

	pauseTitleEntity_ = registry.GenerateEntity();
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

	pauseCursorEntity_ = registry.GenerateEntity();
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