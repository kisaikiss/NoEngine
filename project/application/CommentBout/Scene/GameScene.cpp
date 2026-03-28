#include "stdafx.h"
#include "GameScene.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Data/RailDataIO.h"
#include "application/CommentBout/Component/PlayerComponent.h"
#include "application/CommentBout/Component/PlayerAttackComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Component/InvincibleComponent.h"
#include "application/CommentBout/Component/PlayerHitboxComponent.h"
#include "application/CommentBout/Data/PlayerConfig.h"
#include "application/CommentBout/Data/PlayerDataIO.h"
#include "application/CommentBout/Component/OutGame/OutGameComponets.h"	//アウトゲーム関連
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/Component/HpBarComponent.h"
#include "application/CommentBout/Component/RailCameraComponent.h"
#include "application/CommentBout/Component/RailProgressBarComponent.h"
#include "application/CommentBout/Component/EnemyComponent.h"
#include "application/CommentBout/Utility/CBCollisionMask.h"
#include "application/CommentBout/Utility/CBSpriteLayer.h"
#include "application/CommentBout/Utility/CBGameAudio.h"
#include "application/CommentBout/System/InputHelperSystem.h"
#include "application/CommentBout/System/PlayerControlSystem.h"
#include "application/CommentBout/System/PlayerInfoDebugSystem.h"
#include "application/CommentBout/FieldObject/System/FieldEditorSystem.h"
#include "application/CommentBout/System/LifetimeSystem.h"
#include "application/CommentBout/System/OutGame/PauseSystem.h"
#include "application/CommentBout/System/OutGame/PauseViewSystem.h"
#include "application/CommentBout/System/OutGame/OptionSystem.h"
#include "application/CommentBout/System/OutGame/OptionViewSystem.h"
#include "application/CommentBout/System/RailCameraSystem.h"
#include "application/CommentBout/System/RailProgressBarSystem.h"
#include "application/CommentBout/System/EnemySpawnSystem.h"
#include "application/CommentBout/System/EnemyShootSystem.h"
#include "application/CommentBout/System/EnemyMoveSystem.h"
#include "application/CommentBout/System/BossBehaviorSystem.h"
#include "application/CommentBout/System/EnemyBulletHitSystem.h"
#include "application/CommentBout/System/EnemyContactDamageSystem.h"
#include "application/CommentBout/System/EnemyVisualSystem.h"
#include "application/CommentBout/System/HpBarViewSystem.h"
#include "application/CommentBout/System/SpeechBubbleToBossSystem.h"
#include "application/CommentBout/System/EnemyConfigEditorSystem.h"
#include "application/CommentBout/System/SpeechBubbleConfigEditorSystem.h"
#include "application/CommentBout/Data/SpeechBubbleConfig.h"
#include "application/CommentBout/Data/SpeechBubbleDataIO.h"
#include "application/CommentBout/System/PlayerAttackResolveSystem.h"
#include "application/CommentBout/System/DamageApplySystem.h"
#include "application/CommentBout/System/DamageFlashSystem.h"
#include "application/CommentBout/Spawner/OptionMenuSpawner.h"
#include "application/CommentBout/Spawner/PauseMenuSpawner.h"
#include "application/CommentBout/Collision/System/CollisionSystem.h"
#include "application/CommentBout/Collision/System/CollisionDebugRenderSystem.h"
#include "application/CommentBout/Collision/Component/Collider2DComponent.h"
#include "application/CommentBout/Collision/Component/Collider3DComponent.h"
#include "application/CommentBout/Collision/Component/ProjectedColliderComponent.h"
#include "engine/Runtime/GraphicsCore.h"
#include <algorithm>
#include <cstdio>

void GameScene::Setup() {

	// ----システム登録・--------------------------------------------- //
	// Input / Camera
	//  - PauseSystem
	//  - PlayerControlSystem
	//  - DebugCameraSystem / RailCameraSystem / CameraSystem
	// Spawn / Behavior
	//  - EnemySpawnSystem
	//  - BossBehaviorSystem
	//  - EnemyMoveSystem
	//  - EnemyShootSystem
	// Collision / Resolve
	//  - CollisionSystem
	//  - EnemyBulletHitSystem
	//  - EnemyContactDamageSystem
	//  - PlayerAttackResolveSystem
	// Damage / UI
	//  - SpeechBubbleToBossSystem
	//  - DamageApplySystem
	//  - DamageFlashSystem (must run after DamageApplySystem)
	//  - HpBarViewSystem
	//  - EnemyVisualSystem
	// Others
	//  - LifetimeSystem / OptionSystem / PauseViewSystem / OptionViewSystem / EditSystem
	// --------------------------------------------------------------------------- //

	AddSystem(std::make_unique<InputHelperSystem>());
	AddSystem(std::make_unique<PauseSystem>());
	AddSystem(std::make_unique<PlayerControlSystem>());
	AddSystem(std::make_unique<PlayerInfoDebugSystem>());
	AddSystem(std::make_unique<No::DebugCameraSystem>());
	AddSystem(std::make_unique<RailCameraSystem>());
	AddSystem(std::make_unique<RailProgressBarSystem>());
	AddSystem(std::make_unique<EnemySpawnSystem>());
	AddSystem(std::make_unique<No::CameraSystem>());
	AddSystem(std::make_unique<BossBehaviorSystem>());
	AddSystem(std::make_unique<EnemyMoveSystem>());
	AddSystem(std::make_unique<EnemyShootSystem>());
	AddSystem(std::make_unique<FieldEditorSystem>());
	AddSystem(std::make_unique<CommentBoutCollision::CollisionSystem>());
	AddSystem(std::make_unique<CommentBoutCollision::CollisionDebugRenderSystem>());
	AddSystem(std::make_unique<EnemyBulletHitSystem>());
	AddSystem(std::make_unique<EnemyContactDamageSystem>());
	AddSystem(std::make_unique<PlayerAttackResolveSystem>());
	AddSystem(std::make_unique<SpeechBubbleToBossSystem>());
	AddSystem(std::make_unique<EnemyConfigEditorSystem>());
	AddSystem(std::make_unique<SpeechBubbleConfigEditorSystem>());
	AddSystem(std::make_unique<DamageApplySystem>());
	AddSystem(std::make_unique<DamageFlashSystem>());
	AddSystem(std::make_unique<HpBarViewSystem>());
	AddSystem(std::make_unique<EnemyVisualSystem>());
	AddSystem(std::make_unique<LifetimeSystem>());
	AddSystem(std::make_unique<OptionSystem>());
	AddSystem(std::make_unique<PauseViewSystem>());
	AddSystem(std::make_unique<OptionViewSystem>());
	AddSystem(std::make_unique<No::EditSystem>());

	No::Registry& registry = *GetRegistry();
	CommentBout::GameAudio::InitializeForCommentBout();
	CommentBout::GameAudio::StopTestBGM();
	CommentBout::GameAudio::PlayTestBGM(true);

	//ゲームリソース
	auto gameResourceEntity = registry.GenerateEntity();
	registry.AddComponent<CBGameResourceTag>(gameResourceEntity);
	auto* gameResource = registry.AddComponent<GameResourceComponent>(gameResourceEntity);
	InitializeCommentBoutGameResources(*gameResource);

	auto playerConfigEntity = registry.GenerateEntity();
	registry.AddComponent<CBPlayerConfigTag>(playerConfigEntity);
	auto* playerConfig = registry.AddComponent<PlayerConfig>(playerConfigEntity);
	*playerConfig = PlayerDataIO::Load();

	// 吹き出し設定エンティティ
	auto sbConfigEntity = registry.GenerateEntity();
	registry.AddComponent<CBSpeechBubbleConfigTag>(sbConfigEntity);
	auto* sbConfig = registry.AddComponent<SpeechBubbleConfig>(sbConfigEntity);
	*sbConfig = SpeechBubbleDataIO::Load();

	auto bossHpBarEntity = registry.GenerateEntity();
	registry.AddComponent<CBBossHpBarTag>(bossHpBarEntity);
	auto* bossHpBar = registry.AddComponent<HpBarComponent>(bossHpBarEntity);
	bossHpBar->anchor = { 640.0f, 64.0f };
	bossHpBar->size = { 420.0f, 28.0f };
	bossHpBar->layer = static_cast<int>(CommentBout::ToLayer(CommentBout::SpriteLayer::HpBar));
	bossHpBar->orderBase = 100;

	auto playerHpBarEntity = registry.GenerateEntity();
	registry.AddComponent<CBPlayerHpBarTag>(playerHpBarEntity);
	auto* playerHpBar = registry.AddComponent<HpBarComponent>(playerHpBarEntity);
	playerHpBar->anchor = { 170.0f, 670.0f };
	playerHpBar->size = { 280.0f, 22.0f };
	playerHpBar->fillColor = No::Color(0.2f, 0.7f, 1.0f, 0.95f);
	playerHpBar->layer = static_cast<int>(CommentBout::ToLayer(CommentBout::SpriteLayer::HpBar));
	playerHpBar->orderBase = 100;

	auto railProgressBarEntity = registry.GenerateEntity();
	registry.AddComponent<CBRailProgressBarTag>(railProgressBarEntity);
	auto* railProgressBar = registry.AddComponent<RailProgressBarComponent>(railProgressBarEntity);
	railProgressBar->layer = static_cast<int>(CommentBout::ToLayer(CommentBout::SpriteLayer::RailProgressBar));
	railProgressBar->orderBase = 100;
	railProgressBar->startPosition = { 230.0f, 23.0f };
	railProgressBar->goalPosition = { 1050.0f, 23.0f };
	railProgressBar->barHeight = 10.0f;
	railProgressBar->markerSize = { 22.0f, 22.0f };


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
	registry.AddComponent<PauseMenuConfigComponent>(pauseConfigEntity);
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
	registry.AddComponent<OptionMenuConfigComponent>(optionConfigEntity);
	registry.AddComponent<No::EditTag>(optionConfigEntity)->name = "OptionMenuConfig";


	PauseMenuSpawner::Create(registry, *gameResource);
	OptionMenuSpawner::Create(registry, *gameResource);


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
	auto* cameraTransform = registry.AddComponent<No::TransformComponent>(camera);
	cameraTransform->translate.z = -5.f;
	activeCameraEntity_ = camera;
	debugCameraEntity_ = camera;
	registry.AddComponent<No::EditTag>(camera)->name = "camera";

	railCameraEntity_ = registry.GenerateEntity();
	registry.AddComponent<No::CameraComponent>(railCameraEntity_);
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
	auto* playerComp = registry.AddComponent<PlayerComponent>(playerEntity);
	playerComp->moveSpeed = playerConfig->moveSpeed;
	playerComp->acceleration = playerConfig->acceleration;
	playerComp->deceleration = playerConfig->deceleration;
	playerComp->maxSpeed = playerConfig->maxSpeed;
	playerComp->invincibleDurationDefault = playerConfig->invincibleDurationDefault;
	auto* playerAttack = registry.AddComponent<PlayerAttackComponent>(playerEntity);
	playerAttack->spawnOffset = playerConfig->attackSpawnOffset;
	playerAttack->attackSize = playerConfig->attackSize;
	playerAttack->visibleTime = playerConfig->attackVisibleTime;
	playerAttack->attackLayer = playerConfig->attackLayer;
	playerAttack->attackPower = playerConfig->attackPower;
	auto* playerCommonHealth = registry.AddComponent<HealthComponent>(playerEntity);
	playerCommonHealth->hp = playerConfig->playerMaxHp;
	playerCommonHealth->maxHp = playerConfig->playerMaxHp;
	playerCommonHealth->isDead = false;
	playerCommonHealth->lastDamageTaken = 0;
	auto* playerInvincible = registry.AddComponent<InvincibleComponent>(playerEntity);
	playerInvincible->time = 0.0f;
	playerInvincible->duration = playerConfig->invincibleDuration;
	auto* playerStartTransform = registry.AddComponent<No::StartTransform2DComponent>(playerEntity);
	playerStartTransform->translate = playerConfig->startPosition;
	playerStartTransform->rotation = playerConfig->startRotation;
	playerStartTransform->scale = playerConfig->startScale;
	auto* playerTransform = registry.AddComponent<No::Transform2DComponent>(playerEntity);
	playerTransform->translate = playerStartTransform->translate;
	playerTransform->rotation = playerStartTransform->rotation;
	playerTransform->scale = playerStartTransform->scale;
	auto* playerSprite = registry.AddComponent<No::SpriteComponent>(playerEntity);
	playerSprite->layer = CommentBout::ToLayer(CommentBout::SpriteLayer::Gameplay);
	playerSprite->color = { 1.f, 1.f, 1.f, 0.5f };
	playerSprite->textureHandle = GetGameTextureOrWhite(*gameResource, CommentBoutResourceKey::kPlayerSprite);
	// Phase13: 接触イベントへ段階移行するため、自機にも2Dコライダーを持たせる。
	auto* playerCollider2D = registry.AddComponent<CommentBoutCollision::Collider2DComponent>(playerEntity);
	playerCollider2D->useTransformAsSize = true;
	playerCollider2D->sizeMultiplier = { 1.0f, 1.0f };
	playerCollider2D->collisionLayer = CommentBout::CollisionLayer::CBPlayer;
	playerCollider2D->collisionMask = CommentBout::CollisionMask::CBPlayer;
	playerHpBar->targetEntity = playerEntity;

	auto playerHitboxEntity = registry.GenerateEntity();
	registry.AddComponent<CBPlayerHitboxTag>(playerHitboxEntity);
	auto* playerHitboxComp = registry.AddComponent<PlayerHitboxComponent>(playerHitboxEntity);
	playerHitboxComp->playerEntity = playerEntity;
	playerHitboxComp->useCameraGateForPlayerHit = playerConfig->useCameraGateForPlayerHit;
	playerHitboxComp->cameraGateNear = playerConfig->cameraGateNear;
	playerHitboxComp->cameraGateDepth = playerConfig->cameraGateDepth;
	playerHitboxComp->cameraGateHalfWidth = playerConfig->cameraGateHalfWidth;
	playerHitboxComp->cameraGateHalfHeight = playerConfig->cameraGateHalfHeight;

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
