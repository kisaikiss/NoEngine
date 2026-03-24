#include "stdafx.h"
#include "GameScene.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Data/RailDataIO.h"
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
#include "application/CommentBout/Component/RailCameraComponent.h"
#include "application/CommentBout/Component/EnemyComponent.h"
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
#include "application/CommentBout/System/RailCameraSystem.h"
#include "application/CommentBout/System/EnemySystem.h"
#include "application/CommentBout/Spawner/OptionMenuSpawner.h"
#include "application/CommentBout/Spawner/PauseMenuSpawner.h"
#include "application/TestApp/System/CollisionTestSystem.h"
#include "application/TestApp/Component/Collider2DComponent.h"
#include "application/TestApp/Component/Collider3DComponent.h"
#include "application/TestApp/Component/ProjectedColliderComponent.h"
#include "engine/Runtime/GraphicsCore.h"
#include <algorithm>
#include <cstdio>

void GameScene::Setup() {

	grassNameIndex_ = 0;

	// ---- システム登録（順序が重要）----------------------------------------------　//
	// 1. PlayerControlSystem : 入力処理・攻撃エンティティのスポawn
	// 2. CollisionTestSystem : コライダー更新・投影・衝突判定→ projected->isColliding をセット
	// 3. GrassReactionSystem : 衝突開始を検出しエフェクトエンティティをスポーン→ HitBalloonComponent を付与
	// 4. HitBalloonSystem    : HitBalloonComponent が指す投影位置からエフェクトの Transform2D を毎フレーム更新
	// 5. LifetimeSystem      : 時間切れエンティティを削除	
	// --------------------------------------------------------------------------- //

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
	AddSystem(std::make_unique<EnemySystem>());
	AddSystem(std::make_unique<RailCameraSystem>());
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

	auto optionConfigEntity = registry.GenerateEntity();
	registry.AddComponent<CBOptionConfigTag>(optionConfigEntity);
	registry.AddComponent<OptionMenuConfigComponent>(optionConfigEntity); // デフォルト値 = JSON値
	registry.AddComponent<No::EditTag>(optionConfigEntity)->name = "OptionMenuConfig";


	PauseMenuSpawner::Create(registry, whiteTexture);
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
	debugCameraEntity_ = camera;

	railCameraEntity_ = registry.GenerateEntity();
	registry.AddComponent<No::CameraComponent>(railCameraEntity_);
	registry.AddComponent<No::EditTag>(railCameraEntity_)->name = "rail_camera";
	auto* railCameraTransform = registry.AddComponent<No::TransformComponent>(railCameraEntity_);
	railCameraTransform->translate = { 0.0f, 2.0f, -10.0f };
	auto* railCamera = registry.AddComponent<RailCameraComponent>(railCameraEntity_);
	railCamera->stageName = "Stage_01";
	railCamera->railFilePath = MakeRailFilePath(railCamera->stageName);
	if (!LoadEventsToComponent(*railCamera, railCamera->stageName)) {
		RailEventData stopEvent;
		stopEvent.type = RailEventType::RailStop;
		stopEvent.triggerDistance = 8.0f;
		railCamera->events.push_back(stopEvent);

		RailEventData resumeEvent;
		resumeEvent.type = RailEventType::RailResume;
		resumeEvent.triggerDistance = 8.0f;
		resumeEvent.resumeCondition = RailResumeConditionType::AfterSeconds;
		resumeEvent.resumeAfterSeconds = 2.0f;
		railCamera->events.push_back(resumeEvent);

		railCamera->selectedEventIndex = 0;
	}

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
	RailCameraImGui();
	RailEditorImGui();
	ChangeSceneImGui();

}

void GameScene::CameraImGui()
{
#ifdef USE_IMGUI
	ImGui::Begin("CameraControl");
	if (ImGui::Button("Use Debug Camera")) {
		GetRegistry()->AddComponent<No::ActiveCameraTag>(debugCameraEntity_);
		activeCameraEntity_ = debugCameraEntity_;
	}
	if (ImGui::Button("Use Rail Camera")) {
		GetRegistry()->AddComponent<No::ActiveCameraTag>(railCameraEntity_);
		activeCameraEntity_ = railCameraEntity_;
	}
	ImGui::Text("Active: %s", (activeCameraEntity_ == railCameraEntity_) ? "Rail" : "Debug");
	ImGui::End();
#endif
}

void GameScene::RailCameraImGui()
{
	railCameraEditor_.DrawRailCameraImGui(GetRegistry(), railCameraEntity_, &ResetEventRuntime);
}

void GameScene::RailEditorImGui()
{
#ifdef USE_IMGUI
	if (railCameraEntity_ == No::nullEntity) {
		return;
	}

	auto* rail = GetRegistry()->GetComponent<RailCameraComponent>(railCameraEntity_);
	if (!rail) {
		return;
	}

	static char stageNameBuffer[64] = "";
	if (stageNameBuffer[0] == '\0') {
		std::snprintf(stageNameBuffer, sizeof(stageNameBuffer), "%s", rail->stageName.c_str());
	}

	ImGui::Begin("Stage IO");
	ImGui::InputText("StageName", stageNameBuffer, sizeof(stageNameBuffer));

	const std::string stageName(stageNameBuffer);
	if (ImGui::Button("Load Rail") && !stageName.empty()) {
		LoadRailToComponent(*rail, stageName);
	}
	ImGui::SameLine();
	if (ImGui::Button("Save Rail") && !stageName.empty()) {
		rail->stageName = stageName;
		rail->railFilePath = MakeRailFilePath(stageName);
		SaveRailToJson(*rail, stageName);
	}

	if (ImGui::Button("Load Events") && !stageName.empty()) {
		LoadEventsToComponent(*rail, stageName);
	}
	ImGui::SameLine();
	if (ImGui::Button("Save Events") && !stageName.empty()) {
		rail->stageName = stageName;
		SaveEventsToJson(*rail, stageName);
	}
	ImGui::End();
#endif

	railCameraEditor_.DrawRailEditorImGui(GetRegistry(), railCameraEntity_);
	gameEventEditor_.DrawGameEventEditorImGui(GetRegistry(), railCameraEntity_);
}

void GameScene::ChangeSceneImGui()
{
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