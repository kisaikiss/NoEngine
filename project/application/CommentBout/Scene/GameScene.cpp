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
#include "application/CommentBout/Component/GameResultComponent.h"
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
#include "application/CommentBout/System/RailCameraEditorSystem.h"
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
#include "application/CommentBout/Data/SpeechBubbleConfig.h"
#include "application/CommentBout/Data/SpeechBubbleDataIO.h"
#include "application/CommentBout/System/PlayerAttackResolveSystem.h"
#include "application/CommentBout/System/DamageApplySystem.h"
#include "application/CommentBout/System/DamageFlashSystem.h"
#include "application/CommentBout/System/PlayerAnimSystem.h"
#include "application/CommentBout/System/GameResultSystem.h"
#include "application/CommentBout/System/BossDefeatSystem.h"
#include "application/CommentBout/System/ClearOverSystem.h"
#include "application/CommentBout/System/ClearOverViewSystem.h"
#include "application/CommentBout/Component/PlayerAnimStateComponent.h"
#include "application/CommentBout/Component/BossDefeatSequenceComponent.h"
#include "application/CommentBout/Component/ClearOverStateComponent.h"
#include "application/CommentBout/Component/ClearOverConfigComponent.h"
#include "application/CommentBout/Component/ClearOverViewComponent.h"
#include "application/CommentBout/Data/ClearOverConfigDataIO.h"
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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4702)
#endif

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
	AddSystem(std::make_unique<RailCameraEditorSystem>());
	AddSystem(std::make_unique<RailProgressBarSystem>());
	AddSystem(std::make_unique<EnemySpawnSystem>());
	AddSystem(std::make_unique<No::CameraSystem>());
	AddSystem(std::make_unique<BossBehaviorSystem>());
	AddSystem(std::make_unique<EnemyMoveSystem>());
	AddSystem(std::make_unique<EnemyShootSystem>());
	AddSystem(std::make_unique<FieldEditorSystem>(&editorManager_.GetFieldObjectEditor()));
	AddSystem(std::make_unique<CommentBoutCollision::CollisionSystem>());
	AddSystem(std::make_unique<CommentBoutCollision::CollisionDebugRenderSystem>());
	AddSystem(std::make_unique<EnemyBulletHitSystem>());
	AddSystem(std::make_unique<EnemyContactDamageSystem>());
	AddSystem(std::make_unique<PlayerAttackResolveSystem>());
	AddSystem(std::make_unique<SpeechBubbleToBossSystem>());
	AddSystem(std::make_unique<DamageApplySystem>());
	AddSystem(std::make_unique<DamageFlashSystem>());
	AddSystem(std::make_unique<PlayerAnimSystem>());
	AddSystem(std::make_unique<No::SpriteAnimationSystem>());
	AddSystem(std::make_unique<GameResultSystem>());
	AddSystem(std::make_unique<BossDefeatSystem>());
	AddSystem(std::make_unique<ClearOverSystem>());
	AddSystem(std::make_unique<ClearOverViewSystem>());
	AddSystem(std::make_unique<HpBarViewSystem>());
	AddSystem(std::make_unique<EnemyVisualSystem>());
	AddSystem(std::make_unique<LifetimeSystem>());
	AddSystem(std::make_unique<OptionSystem>());
	AddSystem(std::make_unique<PauseViewSystem>());
	AddSystem(std::make_unique<OptionViewSystem>());
	AddSystem(std::make_unique<No::EditSystem>());

	No::Registry& registry = *GetRegistry();
	CommentBout::GameAudio::InitializeForCommentBout();
	CommentBout::GameAudio::StopBGMClip(CommentBoutResourceKey::kBGMTitle);
	CommentBout::GameAudio::PlayBGMClip(CommentBoutResourceKey::kBGMInGame, true);

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


	auto gameResultEntity = registry.GenerateEntity();
	registry.AddComponent<CBGameResultTag>(gameResultEntity);
	registry.AddComponent<GameResultComponent>(gameResultEntity);

	auto bossDefeatEntity = registry.GenerateEntity();
	registry.AddComponent<CBBossDefeatTag>(bossDefeatEntity);
	registry.AddComponent<BossDefeatSequenceComponent>(bossDefeatEntity);

	auto clearOverStateEntity = registry.GenerateEntity();
	registry.AddComponent<CBClearOverStateTag>(clearOverStateEntity);
	registry.AddComponent<ClearOverStateComponent>(clearOverStateEntity);

	auto clearOverConfigEntity = registry.GenerateEntity();
	registry.AddComponent<CBClearOverConfigTag>(clearOverConfigEntity);
	auto* clearOverConfig = registry.AddComponent<ClearOverConfigComponent>(clearOverConfigEntity);
	*clearOverConfig = ClearOverConfigDataIO::Load();

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
	registry.AddComponent<No::CameraComponent>(camera);
	registry.AddComponent<No::DebugCameraComponent>(camera);
	auto* cameraTransform = registry.AddComponent<No::TransformComponent>(camera);
	cameraTransform->translate.z = -5.f;
	activeCameraEntity_ = camera;
	debugCameraEntity_ = camera;
	registry.AddComponent<No::EditTag>(camera)->name = "camera";

	railCameraEntity_ = registry.GenerateEntity();
	registry.AddComponent<No::ActiveCameraTag>(railCameraEntity_);
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

	editorManager_.Initialize(registry, railCameraEntity_);

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
	playerSprite->color = { 1.f, 1.f, 1.f, 1.f };
	playerSprite->textureHandle = GetGameTextureOrWhite(*gameResource, CommentBoutResourceKey::kPlayerSprite);
	auto* playerAnimator = registry.AddComponent<No::Animator2DComponent>(playerEntity);
	playerAnimator->animeFrameWidth  = playerConfig->animFrameSize;
	playerAnimator->animeFrameHeight = playerConfig->animFrameSize;
	playerAnimator->currentAnimation = 0;
	playerAnimator->framesNum        = static_cast<uint32_t>(std::max(1, playerConfig->animIdleFrameCount));
	playerAnimator->frameByFrameTime = playerConfig->animIdleFrameTime;
	registry.AddComponent<PlayerAnimStateComponent>(playerEntity);
	// 移行するため、自機にも2Dコライダーを持たせる。
	auto* playerCollider2D = registry.AddComponent<CommentBoutCollision::Collider2DComponent>(playerEntity);
	playerCollider2D->useTransformAsSize = true;
	playerCollider2D->localOffset    = playerConfig->playerCollider2D.localOffset2D;
	playerCollider2D->sizeMultiplier = playerConfig->playerCollider2D.sizeMultiplier2D;
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
	editorManager_.DrawImGui(*GetRegistry());
	ChangeSceneImGui();
	ClearOverConfigImGui();
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

void GameScene::ClearOverConfigImGui()
{
#ifdef USE_IMGUI
	ClearOverConfigComponent* config = nullptr;
	auto clearOverConfigView = GetRegistry()->View<CBClearOverConfigTag, ClearOverConfigComponent>();
	for (auto e : clearOverConfigView) {
		config = GetRegistry()->GetComponent<ClearOverConfigComponent>(e);
		break;
	}
	if (!config) {
		return;
	}

	ImGui::Begin("ClearOverConfig");

	if (ImGui::CollapsingHeader("Fade")) {
		ImGui::DragFloat("fadeDuration",    &config->fadeDuration,    0.01f, 0.f, 5.f);
		ImGui::DragFloat("fadeTargetAlpha", &config->fadeTargetAlpha, 0.01f, 0.f, 1.f);
	}
	if (ImGui::CollapsingHeader("Logo")) {
		ImGui::DragFloat2("logoStartPos",       &config->logoStartPos.x, 1.f);
		ImGui::DragFloat2("logoEndPos",         &config->logoEndPos.x,   1.f);
		ImGui::DragFloat2("logoSize",           &config->logoSize.x,     1.f);
		ImGui::DragFloat("logoAppearDuration",  &config->logoAppearDuration, 0.01f, 0.f, 5.f);
		ImGui::DragInt("logoEaseType",          &config->logoEaseType, 1, 0, 10);
	}
	if (ImGui::CollapsingHeader("Menu")) {
		ImGui::DragFloat2("itemBaseStartPos",   &config->itemBaseStartPos.x, 1.f);
		ImGui::DragFloat2("itemBaseEndPos",     &config->itemBaseEndPos.x,   1.f);
		ImGui::DragFloat2("itemSize",           &config->itemSize.x,         1.f);
		ImGui::DragFloat("itemSpacing",         &config->itemSpacing,         1.f);
		ImGui::DragFloat("menuAppearDuration",  &config->menuAppearDuration,  0.01f, 0.f, 5.f);
	}
	if (ImGui::CollapsingHeader("Confirm Animation")) {
		ImGui::DragFloat("confirmDuration", &config->confirmDuration, 0.01f, 0.f, 1.f);
		ImGui::DragFloat("selectedScale",   &config->selectedScale,   0.01f, 1.f, 2.f);
		ImGui::DragFloat("confirmScale",    &config->confirmScale,    0.01f, 1.f, 2.f);
	}

	ImGui::Separator();
	if (ImGui::Button("Save JSON")) {
		ClearOverConfigDataIO::Save(*config);
	}
	ImGui::SameLine();
	if (ImGui::Button("Load JSON")) {
		*config = ClearOverConfigDataIO::Load();
	}

	ImGui::End();
#endif
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
