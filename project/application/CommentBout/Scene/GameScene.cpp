#include "stdafx.h"
#include "GameScene.h"
#include <vector>
#include <utility>
#include <array>
#include <algorithm>
#include <cmath>
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Component/PlayerComponent.h"
#include "application/CommentBout/Component/PlayerAttackComponent.h"
#include "application/CommentBout/Component/PauseStateComponent.h"
#include "application/CommentBout/Component/PauseMenuConfigComponent.h"
#include "application/CommentBout/Component/OptionStateComponent.h"
#include "application/CommentBout/Component/OptionMenuConfigComponent.h"
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
	// 5. LifetimeSystem         : 時間切れエンティティを削除
	// ---------------------------------------------------------------------------
	AddSystem(std::make_unique<PlayerControlSystem>());
	AddSystem(std::make_unique<TestApp::CollisionTestSystem>());
	AddSystem(std::make_unique<GrassReactionSystem>());
	AddSystem(std::make_unique<HitBalloonSystem>());
	AddSystem(std::make_unique<LifetimeSystem>());
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
	UpdatePauseState();
	UpdatePauseDim();
	UpdatePauseMenuSprites();
	UpdateOptionState();
	UpdateOptionSprites();
	DrawAudioTestImGui();
	DrawPauseMenu();
	CameraImGui();

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
}

namespace {
	float Clamp01(float v) {
		if (v < 0.0f) {
			return 0.0f;
		}
		if (v > 1.0f) {
			return 1.0f;
		}
		return v;
	}

	No::Vector2 LerpVec2(const No::Vector2& a, const No::Vector2& b, float t) {
		t = Clamp01(t);
		return No::Vector2(
			a.x + (b.x - a.x) * t,
			a.y + (b.y - a.y) * t
		);
	}

	float GetMenuMotionT(const PauseStateComponent* pauseState) {
		if (!pauseState) {
			return 0.0f;
		}
		const float duration = (pauseState->phaseDuration <= 0.0001f) ? 0.0001f : pauseState->phaseDuration;
		switch (pauseState->phase) {
		case PauseStateComponent::Opening:
		{
			const float t = pauseState->phaseTime / duration;
			return No::EaseOutBack(0.0f, 1.0f, t);
		}
		case PauseStateComponent::Closing:
		{
			const float t = pauseState->phaseTime / duration;
			return 1.0f - No::EaseOutCubic(0.0f, 1.0f, t);
		}
		case PauseStateComponent::Open:
		case PauseStateComponent::OptionOpening:
		case PauseStateComponent::OptionOpen:
		case PauseStateComponent::OptionClosing:
			return 1.0f;
		case PauseStateComponent::Closed:
		default:
			return 0.0f;
		}
	}

	float GetPauseDeltaTime() {
#ifdef USE_IMGUI
		return std::max(0.0f, ImGui::GetIO().DeltaTime);
#else
		return 1.0f / 60.0f;
#endif
	}

	float EaseByType(int easeType, float t) {
		t = Clamp01(t);
		switch (easeType) {
		case 1:
			return No::EaseInExpo(0.0f, 1.0f, t);
		case 2:
			return No::EaseOutCubic(0.0f, 1.0f, t);
		default:
			return No::EaseInOutSine(0.0f, 1.0f, t);
		}
	}

	float SafeDuration(float d) {
		return (d <= 0.0001f) ? 0.0001f : d;
	}

	void StartPhase(PauseStateComponent* state, int phase, float duration) {
		state->phase = phase;
		state->phaseTime = 0.0f;
		state->phaseDuration = SafeDuration(duration);
	}

	void StartOptionPhase(OptionStateComponent* state, int phase, float duration) {
		state->phase = phase;
		state->phaseTime = 0.0f;
		state->phaseDuration = (duration <= 0.0001f) ? 0.0001f : duration;
	}
}

void GameScene::UpdatePauseState()
{
	No::Registry& registry = *GetRegistry();
	PauseStateComponent* pauseState = nullptr;
	PauseMenuConfigComponent* pauseConfig = nullptr;
	OptionStateComponent* optionState = nullptr;
	OptionMenuConfigComponent* optionConfig = nullptr;

	auto pauseView = registry.View<CBPauseStateTag, PauseStateComponent>();
	for (auto entity : pauseView) {
		pauseState = registry.GetComponent<PauseStateComponent>(entity);
		if (pauseState) {
			break;
		}
	}

	auto configView = registry.View<CBPauseConfigTag, PauseMenuConfigComponent>();
	for (auto entity : configView) {
		pauseConfig = registry.GetComponent<PauseMenuConfigComponent>(entity);
		if (pauseConfig) {
			break;
		}
	}

	auto optionStateView = registry.View<CBOptionStateTag, OptionStateComponent>();
	for (auto entity : optionStateView) {
		optionState = registry.GetComponent<OptionStateComponent>(entity);
		if (optionState) {
			break;
		}
	}

	auto optionConfigView = registry.View<CBOptionConfigTag, OptionMenuConfigComponent>();
	for (auto entity : optionConfigView) {
		optionConfig = registry.GetComponent<OptionMenuConfigComponent>(entity);
		if (optionConfig) {
			break;
		}
	}

	if (!pauseState || !pauseConfig) {
		return;
	}

	pauseState->justEnteredPause = false;
	pauseState->justExitedPause = false;

	if (pauseState->isConfirmAnimating) {
		pauseState->confirmAnimTime += GetPauseDeltaTime();
		const float confirmDuration = SafeDuration(pauseConfig->confirmDuration);
		if (pauseState->confirmAnimTime >= confirmDuration) {
			pauseState->isConfirmAnimating = false;
			pauseState->confirmAnimTime = 0.0f;
			pauseState->confirmIndex = -1;

			switch (pauseState->requestedAction) {
			case PauseStateComponent::Resume:
				StartPhase(pauseState, PauseStateComponent::Closing, pauseConfig->closeDuration);
				break;
			case PauseStateComponent::Restart:
			{
				No::SceneChangeEvent event;
				event.nextScene = "GameScene";
				registry.EmitEvent(event);
			}
			break;
			case PauseStateComponent::OpenOption:
				if (optionState && optionConfig) {
					optionState->isOpen = true;
					optionState->isEditing = false;
					optionState->selectedIndex = 0;
					StartOptionPhase(optionState, OptionStateComponent::Opening, optionConfig->openDuration);
				}
				break;
			case PauseStateComponent::BackToTitle:
			{
				No::SceneChangeEvent event;
				event.nextScene = "TitleScene";
				registry.EmitEvent(event);
			}
			break;
			default:
				break;
			}
			pauseState->requestedAction = PauseStateComponent::None;
		}
	}

	if (pauseState->phase == PauseStateComponent::Closed) {
		pauseState->isPaused = false;
		if (No::Keyboard::IsTrigger(VK_RETURN)) {
			pauseState->isPaused = true;
			pauseState->justEnteredPause = true;
			pauseState->selectedIndex = 0;
			pauseState->requestedAction = PauseStateComponent::None;
			pauseState->isConfirmAnimating = false;
			pauseState->confirmAnimTime = 0.0f;
			pauseState->confirmIndex = -1;
			StartPhase(pauseState, PauseStateComponent::Opening, pauseConfig->openDuration);
		}
		return;
	}

	pauseState->isPaused = true;

	if (pauseState->phase == PauseStateComponent::Opening ||
		pauseState->phase == PauseStateComponent::Closing) {
		pauseState->phaseTime += GetPauseDeltaTime();
		if (pauseState->phaseTime >= pauseState->phaseDuration) {
			switch (pauseState->phase) {
			case PauseStateComponent::Opening:
				StartPhase(pauseState, PauseStateComponent::Open, 1.0f);
				pauseState->phaseTime = 0.0f;
				break;
			case PauseStateComponent::Closing:
				StartPhase(pauseState, PauseStateComponent::Closed, 1.0f);
				pauseState->isPaused = false;
				pauseState->justExitedPause = true;
				break;
			default:
				break;
			}
		}
		return;
	}

	if (optionState && optionState->isOpen) {
		return;
	}

	if (pauseState->isConfirmAnimating) {
		return;
	}

	if (pauseState->phase != PauseStateComponent::Open) {
		return;
	}

	if (pauseState->itemCount > 0) {
		if (No::Keyboard::IsTrigger('W') || No::Keyboard::IsTrigger(VK_UP)) {
			pauseState->selectedIndex--;
			if (pauseState->selectedIndex < 0) {
				pauseState->selectedIndex = pauseState->itemCount - 1;
			}
		}
		if (No::Keyboard::IsTrigger('S') || No::Keyboard::IsTrigger(VK_DOWN)) {
			pauseState->selectedIndex++;
			if (pauseState->selectedIndex >= pauseState->itemCount) {
				pauseState->selectedIndex = 0;
			}
		}
	}

	if (No::Keyboard::IsTrigger(VK_SPACE)) {
		int action = PauseStateComponent::None;
		switch (pauseState->selectedIndex) {
		case 0:
			action = PauseStateComponent::Resume;
			break;
		case 1:
			action = PauseStateComponent::Restart;
			break;
		case 2:
			action = PauseStateComponent::OpenOption;
			break;
		case 3:
			action = PauseStateComponent::BackToTitle;
			break;
		default:
			action = PauseStateComponent::None;
			break;
		}

		if (action != PauseStateComponent::None) {
			pauseState->confirmIndex = pauseState->selectedIndex;
			pauseState->confirmAnimTime = 0.0f;
			pauseState->isConfirmAnimating = true;
			pauseState->requestedAction = action;
		}
	}
}

void GameScene::UpdatePauseDim()
{
	No::Registry& registry = *GetRegistry();
	PauseStateComponent* pauseState = nullptr;
	PauseMenuConfigComponent* pauseConfig = nullptr;

	auto pauseView = registry.View<CBPauseStateTag, PauseStateComponent>();
	for (auto entity : pauseView) {
		pauseState = registry.GetComponent<PauseStateComponent>(entity);
		if (pauseState) {
			break;
		}
	}

	auto configView = registry.View<CBPauseConfigTag, PauseMenuConfigComponent>();
	for (auto entity : configView) {
		pauseConfig = registry.GetComponent<PauseMenuConfigComponent>(entity);
		if (pauseConfig) {
			break;
		}
	}

	if (!pauseState || !pauseConfig || pauseDimEntity_ == No::nullEntity) {
		return;
	}

	if (!registry.Has<No::Transform2DComponent>(pauseDimEntity_) || !registry.Has<No::SpriteComponent>(pauseDimEntity_)) {
		return;
	}

	auto* dimTransform = registry.GetComponent<No::Transform2DComponent>(pauseDimEntity_);
	auto* dimSprite = registry.GetComponent<No::SpriteComponent>(pauseDimEntity_);

	auto* mainWindow = NoEngine::GraphicsCore::gWindowManager.GetMainWindow();
	if (mainWindow) {
		const auto& windowSize = mainWindow->GetWindowSize();
		const float width = static_cast<float>(windowSize.clientWidth);
		const float height = static_cast<float>(windowSize.clientHeight);
		dimTransform->translate = { width * 0.5f, height * 0.5f };
		dimTransform->scale = { width, height };
	}

	const float maxAlpha = std::max(0.0f, std::min(1.0f, pauseConfig->dimAlpha));
	float alpha = 0.0f;

	switch (pauseState->phase) {
	case PauseStateComponent::Opening:
	{
		const float t = pauseState->phaseTime / SafeDuration(pauseState->phaseDuration);
		alpha = maxAlpha * EaseByType(pauseConfig->easeType, t);
	}
	break;
	case PauseStateComponent::Closing:
	{
		const float t = pauseState->phaseTime / SafeDuration(pauseState->phaseDuration);
		alpha = maxAlpha * (1.0f - EaseByType(pauseConfig->easeType, t));
	}
	break;
	case PauseStateComponent::OptionOpening:
	{
		const float t = pauseState->phaseTime / SafeDuration(pauseState->phaseDuration);
		alpha = maxAlpha + 0.12f * EaseByType(pauseConfig->easeType, t);
	}
	break;
	case PauseStateComponent::OptionOpen:
		alpha = maxAlpha + 0.12f;
		break;
	case PauseStateComponent::OptionClosing:
	{
		const float t = pauseState->phaseTime / SafeDuration(pauseState->phaseDuration);
		alpha = (maxAlpha + 0.12f) - 0.12f * EaseByType(pauseConfig->easeType, t);
	}
	break;
	case PauseStateComponent::Open:
		alpha = maxAlpha;
		break;
	case PauseStateComponent::Closed:
	default:
		alpha = 0.0f;
		break;
	}

	alpha = std::max(0.0f, std::min(1.0f, alpha));
	dimSprite->layer = static_cast<uint32_t>(std::max(0, pauseConfig->dimLayer));
	dimSprite->isVisible = (alpha > 0.0001f);
	dimSprite->color = { 0.0f, 0.0f, 0.0f, alpha };
}

void GameScene::DrawPauseMenu()
{
#ifdef USE_IMGUI
	No::Registry& registry = *GetRegistry();
	static_cast<void>(registry);
#endif
}

void GameScene::CameraImGui()
{
#ifdef USE_IMGUI
	ImGui::Begin("CameraControl");
	// カメラ切り替えボタン
	if (ImGui::Button("CameraChange")) {
		GetRegistry()->AddComponent<No::ActiveCameraTag>(activeCameraEntity_);
	}
	ImGui::End();
#endif // USE_IMGUI
}

void GameScene::CreatePauseMenuSprites(const NoEngine::TextureRef& whiteTexture)
{
	No::Registry& registry = *GetRegistry();
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
	titleSprite->color = { 1.0f, 1.0f, 1.f, 0.0f };
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
		itemSprite->color = { 1.0f, 1.0f, 1.f, 0.0f };
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
}

void GameScene::CreateOptionSprites(const NoEngine::TextureRef& whiteTexture)
{
	No::Registry& registry = *GetRegistry();

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
}

void GameScene::UpdatePauseMenuSprites()
{
	No::Registry& registry = *GetRegistry();
	PauseStateComponent* pauseState = nullptr;
	PauseMenuConfigComponent* pauseConfig = nullptr;

	auto pauseView = registry.View<CBPauseStateTag, PauseStateComponent>();
	for (auto entity : pauseView) {
		pauseState = registry.GetComponent<PauseStateComponent>(entity);
		if (pauseState) {
			break;
		}
	}

	auto configView = registry.View<CBPauseConfigTag, PauseMenuConfigComponent>();
	for (auto entity : configView) {
		pauseConfig = registry.GetComponent<PauseMenuConfigComponent>(entity);
		if (pauseConfig) {
			break;
		}
	}

	if (!pauseState || !pauseConfig) {
		return;
	}

	const float menuMotionT = Clamp01(GetMenuMotionT(pauseState));
	const float menuVisibility = menuMotionT;

	float confirmPunch = 0.0f;
	if (pauseState->isConfirmAnimating) {
		const float t = pauseState->confirmAnimTime / SafeDuration(pauseConfig->confirmDuration);
		const float eased = EaseByType(pauseConfig->easeType, t);
		confirmPunch = 1.0f - std::fabs(2.0f * eased - 1.0f);
	}

	if (pauseMenuBgEntity_ != No::nullEntity &&
		registry.Has<No::Transform2DComponent>(pauseMenuBgEntity_) &&
		registry.Has<No::SpriteComponent>(pauseMenuBgEntity_)) {
		auto* bgTransform = registry.GetComponent<No::Transform2DComponent>(pauseMenuBgEntity_);
		auto* bgSprite = registry.GetComponent<No::SpriteComponent>(pauseMenuBgEntity_);
		bgTransform->translate = LerpVec2(pauseConfig->menuBgStartPosition, pauseConfig->menuBgEndPosition, menuMotionT);
		bgTransform->scale = LerpVec2(pauseConfig->menuBgStartSize, pauseConfig->menuBgEndSize, menuMotionT);
		bgSprite->layer = static_cast<uint32_t>(std::max(0, pauseConfig->menuBgLayer));
		bgSprite->isVisible = (menuVisibility > 0.0001f);
		bgSprite->color = {
			pauseConfig->menuBgColor.r,
			pauseConfig->menuBgColor.g,
			pauseConfig->menuBgColor.b,
			pauseConfig->menuBgColor.a * menuVisibility
		};
	}

	if (pausePanelLineEntity_ != No::nullEntity &&
		registry.Has<No::Transform2DComponent>(pausePanelLineEntity_) &&
		registry.Has<No::SpriteComponent>(pausePanelLineEntity_)) {
		auto* lineTransform = registry.GetComponent<No::Transform2DComponent>(pausePanelLineEntity_);
		auto* lineSprite = registry.GetComponent<No::SpriteComponent>(pausePanelLineEntity_);
		lineTransform->translate = LerpVec2(pauseConfig->panelLineStartPosition, pauseConfig->panelLineEndPosition, menuMotionT);
		lineTransform->scale = LerpVec2(pauseConfig->panelLineStartSize, pauseConfig->panelLineEndSize, menuMotionT);
		lineSprite->layer = static_cast<uint32_t>(std::max(0, pauseConfig->panelLineLayer));
		lineSprite->isVisible = (menuVisibility > 0.0001f);
		lineSprite->color = {
			pauseConfig->panelLineColor.r,
			pauseConfig->panelLineColor.g,
			pauseConfig->panelLineColor.b,
			pauseConfig->panelLineColor.a * menuVisibility
		};
	}

	if (pauseTitleEntity_ != No::nullEntity &&
		registry.Has<No::Transform2DComponent>(pauseTitleEntity_) &&
		registry.Has<No::SpriteComponent>(pauseTitleEntity_)) {
		auto* titleTransform = registry.GetComponent<No::Transform2DComponent>(pauseTitleEntity_);
		auto* titleSprite = registry.GetComponent<No::SpriteComponent>(pauseTitleEntity_);
		titleTransform->translate = LerpVec2(pauseConfig->titleStartPosition, pauseConfig->titleEndPosition, menuMotionT);
		titleTransform->scale = pauseConfig->titleSize;
		titleSprite->layer = static_cast<uint32_t>(std::max(0, pauseConfig->titleLayer));
		titleSprite->isVisible = (menuVisibility > 0.0001f);
		titleSprite->color = { 1.0f, 1.0f, 1.0f, menuVisibility };
	}

	const No::Vector2 itemBasePosition = LerpVec2(pauseConfig->itemBaseStartPosition, pauseConfig->itemBaseEndPosition, menuMotionT);
	for (size_t i = 0; i < pauseItemEntities_.size(); ++i) {
		const No::Entity e = pauseItemEntities_[i];
		if (e == No::nullEntity) {
			continue;
		}
		if (!registry.Has<No::Transform2DComponent>(e) || !registry.Has<No::SpriteComponent>(e)) {
			continue;
		}

		auto* itemTransform = registry.GetComponent<No::Transform2DComponent>(e);
		auto* itemSprite = registry.GetComponent<No::SpriteComponent>(e);

		const float baseY = itemBasePosition.y + pauseConfig->itemSpacing * static_cast<float>(i);

		float itemScale = 1.0f;
		const bool isSelected = (static_cast<int>(i) == pauseState->selectedIndex && pauseState->phase == PauseStateComponent::Open);
		if (isSelected) {
			itemScale = pauseConfig->selectedScale;
		}
		if (pauseState->isConfirmAnimating && static_cast<int>(i) == pauseState->confirmIndex) {
			itemScale = 1.0f + (pauseConfig->confirmScale - 1.0f) * confirmPunch;
		}

		itemTransform->translate = { itemBasePosition.x, baseY };
		itemTransform->scale = {
			pauseConfig->itemSize.x * itemScale,
			pauseConfig->itemSize.y * itemScale
		};

		itemSprite->layer = static_cast<uint32_t>(std::max(0, pauseConfig->itemLayer));
		itemSprite->orderInLayer = static_cast<uint32_t>(i);
		itemSprite->isVisible = (menuVisibility > 0.0001f);
		if (isSelected || (pauseState->isConfirmAnimating && static_cast<int>(i) == pauseState->confirmIndex)) {
			itemSprite->color = { 1.0f, 1.0f, 1.0f, menuVisibility };
		} else {
			itemSprite->color = { 0.82f, 0.82f, 0.82f, menuVisibility };
		}
	}

	if (pauseCursorEntity_ != No::nullEntity &&
		registry.Has<No::Transform2DComponent>(pauseCursorEntity_) &&
		registry.Has<No::SpriteComponent>(pauseCursorEntity_)) {
		auto* cursorTransform = registry.GetComponent<No::Transform2DComponent>(pauseCursorEntity_);
		auto* cursorSprite = registry.GetComponent<No::SpriteComponent>(pauseCursorEntity_);
		const float cursorY = itemBasePosition.y + pauseConfig->itemSpacing * static_cast<float>(pauseState->selectedIndex);
		const No::Vector2 cursorOffset = LerpVec2(pauseConfig->cursorStartOffset, pauseConfig->cursorEndOffset, menuMotionT);
		cursorTransform->translate = {
			itemBasePosition.x + cursorOffset.x,
			cursorY + cursorOffset.y
		};
		cursorTransform->scale = pauseConfig->cursorSize;
		cursorSprite->layer = static_cast<uint32_t>(std::max(0, pauseConfig->cursorLayer));
		cursorSprite->isVisible = (!pauseState->isConfirmAnimating && pauseState->phase == PauseStateComponent::Open && menuVisibility > 0.0001f);
		cursorSprite->color = { 1.0f, 0.95f, 0.35f, menuVisibility };
	}
}

void GameScene::UpdateOptionSprites()
{
	No::Registry& registry = *GetRegistry();
	OptionStateComponent* optionState = nullptr;
	OptionMenuConfigComponent* optionConfig = nullptr;
	PauseMenuConfigComponent* pauseConfig = nullptr;

	auto stateView = registry.View<CBOptionStateTag, OptionStateComponent>();
	for (auto entity : stateView) {
		optionState = registry.GetComponent<OptionStateComponent>(entity);
		if (optionState) {
			break;
		}
	}
	auto configView = registry.View<CBOptionConfigTag, OptionMenuConfigComponent>();
	for (auto entity : configView) {
		optionConfig = registry.GetComponent<OptionMenuConfigComponent>(entity);
		if (optionConfig) {
			break;
		}
	}
	auto pauseConfigView = registry.View<CBPauseConfigTag, PauseMenuConfigComponent>();
	for (auto entity : pauseConfigView) {
		pauseConfig = registry.GetComponent<PauseMenuConfigComponent>(entity);
		if (pauseConfig) {
			break;
		}
	}
	if (!optionState || !optionConfig) {
		return;
	}

	float t = 0.0f;
	if (optionState->phase == OptionStateComponent::Opening) {
		t = No::EaseOutBack(0.0f, 1.0f, optionState->phaseTime / std::max(0.0001f, optionState->phaseDuration));
	}
	else if (optionState->phase == OptionStateComponent::Closing) {
		t = 1.0f - No::EaseOutCubic(0.0f, 1.0f, optionState->phaseTime / std::max(0.0001f, optionState->phaseDuration));
	}
	else if (optionState->phase == OptionStateComponent::OpenSelect || optionState->phase == OptionStateComponent::OpenEdit) {
		t = 1.0f;
	}
	t = Clamp01(t);

	float confirmPunch = 0.0f;
	if (optionState->isConfirmAnimating) {
		float confirmDuration = optionConfig->confirmDuration;
		int confirmEaseType = 2;
		if (optionState->confirmIndex == 4 && pauseConfig) {
			confirmDuration = pauseConfig->confirmDuration;
			confirmEaseType = pauseConfig->easeType;
		}
		const float tt = optionState->confirmAnimTime / SafeDuration(confirmDuration);
		const float eased = EaseByType(confirmEaseType, tt);
		confirmPunch = 1.0f - std::fabs(2.0f * eased - 1.0f);
	}

	const No::Vector2 basePos = LerpVec2(optionConfig->itemBaseStartPosition, optionConfig->itemBaseEndPosition, t);
	const No::Vector2 backItemPos = LerpVec2(optionConfig->backItemStartPosition, optionConfig->backItemEndPosition, t);

	if (optionDimEntity_ != No::nullEntity && registry.Has<No::Transform2DComponent>(optionDimEntity_) && registry.Has<No::SpriteComponent>(optionDimEntity_)) {
		auto* tr = registry.GetComponent<No::Transform2DComponent>(optionDimEntity_);
		auto* sp = registry.GetComponent<No::SpriteComponent>(optionDimEntity_);
		tr->translate = LerpVec2(optionConfig->dimStartPosition, optionConfig->dimEndPosition, t);
		tr->scale = optionConfig->dimSize;
		tr->rotation = optionConfig->dimRotation;
		sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->dimLayer));
		sp->isVisible = (t > 0.0001f);
		sp->color = { optionConfig->dimColor.r, optionConfig->dimColor.g, optionConfig->dimColor.b, optionConfig->dimColor.a * t };
	}

	if (optionBgEntity_ != No::nullEntity && registry.Has<No::Transform2DComponent>(optionBgEntity_) && registry.Has<No::SpriteComponent>(optionBgEntity_)) {
		auto* tr = registry.GetComponent<No::Transform2DComponent>(optionBgEntity_);
		auto* sp = registry.GetComponent<No::SpriteComponent>(optionBgEntity_);
		tr->translate = LerpVec2(optionConfig->bgStartPosition, optionConfig->bgEndPosition, t);
		tr->scale = LerpVec2(optionConfig->bgStartSize, optionConfig->bgEndSize, t);
		tr->rotation = optionConfig->bgRotation;
		sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->bgLayer));
		sp->isVisible = (t > 0.0001f);
		sp->color = { optionConfig->bgColor.r, optionConfig->bgColor.g, optionConfig->bgColor.b, optionConfig->bgColor.a * t };
	}

	if (optionLineEntity_ != No::nullEntity && registry.Has<No::Transform2DComponent>(optionLineEntity_) && registry.Has<No::SpriteComponent>(optionLineEntity_)) {
		auto* tr = registry.GetComponent<No::Transform2DComponent>(optionLineEntity_);
		auto* sp = registry.GetComponent<No::SpriteComponent>(optionLineEntity_);
		tr->translate = LerpVec2(optionConfig->lineStartPosition, optionConfig->lineEndPosition, t);
		tr->scale = optionConfig->lineSize;
		tr->rotation = optionConfig->lineRotation;
		sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->lineLayer));
		sp->isVisible = (t > 0.0001f);
		sp->color = { optionConfig->lineColor.r, optionConfig->lineColor.g, optionConfig->lineColor.b, optionConfig->lineColor.a * t };
	}

	if (optionTitleEntity_ != No::nullEntity && registry.Has<No::Transform2DComponent>(optionTitleEntity_) && registry.Has<No::SpriteComponent>(optionTitleEntity_)) {
		auto* tr = registry.GetComponent<No::Transform2DComponent>(optionTitleEntity_);
		auto* sp = registry.GetComponent<No::SpriteComponent>(optionTitleEntity_);
		tr->translate = LerpVec2(optionConfig->titleStartPosition, optionConfig->titleEndPosition, t);
		tr->scale = optionConfig->titleSize;
		sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->labelLayer));
		sp->isVisible = (t > 0.0001f);
		sp->color = { 1.0f, 1.0f, 1.0f, t };
	}

	const std::array<float, 3> volumes = { optionState->masterVolume, optionState->bgmVolume, optionState->seVolume };
	for (size_t i = 0; i < optionItemEntities_.size(); ++i) {
		const float rowY = basePos.y + optionConfig->itemSpacing * static_cast<float>(i);
		const No::Entity itemE = optionItemEntities_[i];
		if (itemE != No::nullEntity && registry.Has<No::Transform2DComponent>(itemE) && registry.Has<No::SpriteComponent>(itemE)) {
			auto* tr = registry.GetComponent<No::Transform2DComponent>(itemE);
			auto* sp = registry.GetComponent<No::SpriteComponent>(itemE);
			const bool isBack = (i == 4);
			const No::Vector2 baseSize = isBack ? optionConfig->backItemSize : optionConfig->itemSize;
			const No::Vector2 baseTranslate = isBack ? backItemPos : No::Vector2(basePos.x, rowY);
			float scale = 1.0f;
			if (optionState->isConfirmAnimating && static_cast<int>(i) == optionState->confirmIndex) {
				float confirmScale = optionConfig->confirmScale;
				if (isBack && pauseConfig) {
					confirmScale = pauseConfig->confirmScale;
				}
				scale = 1.0f + (confirmScale - 1.0f) * confirmPunch;
			}
			tr->translate = baseTranslate;
			tr->scale = { baseSize.x * scale, baseSize.y * scale };
			sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->itemLayer));
			sp->orderInLayer = static_cast<uint32_t>(i);
			sp->isVisible = (t > 0.0001f);
			sp->color = { optionConfig->itemColor.r, optionConfig->itemColor.g, optionConfig->itemColor.b, optionConfig->itemColor.a * t };
		}

		const No::Entity labelE = optionLabelEntities_[i];
		if (labelE != No::nullEntity && registry.Has<No::Transform2DComponent>(labelE) && registry.Has<No::SpriteComponent>(labelE)) {
			auto* tr = registry.GetComponent<No::Transform2DComponent>(labelE);
			auto* sp = registry.GetComponent<No::SpriteComponent>(labelE);
			if (i == 4) {
				tr->translate = LerpVec2(optionConfig->backLabelStartPosition, optionConfig->backLabelEndPosition, t);
				tr->scale = optionConfig->backLabelSize;
			}
			else {
				tr->translate = { basePos.x + optionConfig->labelOffset.x, rowY + optionConfig->labelOffset.y };
				tr->scale = optionConfig->labelSize;
			}
			sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->labelLayer));
			sp->isVisible = (t > 0.0001f);
			sp->color = { 1.0f, 1.0f, 1.0f, t };
		}

		if (i < optionBarBaseEntities_.size()) {
			const No::Entity baseE = optionBarBaseEntities_[i];
			const No::Entity fillE = optionBarFillEntities_[i];
			if (baseE != No::nullEntity && registry.Has<No::Transform2DComponent>(baseE) && registry.Has<No::SpriteComponent>(baseE)) {
				auto* tr = registry.GetComponent<No::Transform2DComponent>(baseE);
				auto* sp = registry.GetComponent<No::SpriteComponent>(baseE);
				tr->translate = { basePos.x + optionConfig->barOffset.x, rowY + optionConfig->barOffset.y };
				tr->scale = optionConfig->barBaseSize;
				sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->barBaseLayer));
				sp->isVisible = (t > 0.0001f);
				sp->color = { optionConfig->barBaseColor.r, optionConfig->barBaseColor.g, optionConfig->barBaseColor.b, optionConfig->barBaseColor.a * t };
			}
			if (fillE != No::nullEntity && registry.Has<No::Transform2DComponent>(fillE) && registry.Has<No::SpriteComponent>(fillE)) {
				auto* tr = registry.GetComponent<No::Transform2DComponent>(fillE);
				auto* sp = registry.GetComponent<No::SpriteComponent>(fillE);
				const float fillW = optionConfig->barFillMinSize.x + (optionConfig->barBaseSize.x - optionConfig->barFillMinSize.x) * Clamp01(volumes[i]);
				tr->translate = { basePos.x + optionConfig->barOffset.x - (optionConfig->barBaseSize.x - fillW) * 0.5f, rowY + optionConfig->barOffset.y };
				tr->scale = { fillW, optionConfig->barFillMinSize.y };
				sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->barFillLayer));
				sp->isVisible = (t > 0.0001f);
				sp->color = { optionConfig->barFillColor.r, optionConfig->barFillColor.g, optionConfig->barFillColor.b, optionConfig->barFillColor.a * t };
			}
		}
	}

	if (optionToggleEntity_ != No::nullEntity && registry.Has<No::Transform2DComponent>(optionToggleEntity_) && registry.Has<No::SpriteComponent>(optionToggleEntity_)) {
		auto* tr = registry.GetComponent<No::Transform2DComponent>(optionToggleEntity_);
		auto* sp = registry.GetComponent<No::SpriteComponent>(optionToggleEntity_);
		const float y = basePos.y + optionConfig->itemSpacing * 3.0f;
		tr->translate = { basePos.x + optionConfig->toggleOffset.x, y + optionConfig->toggleOffset.y };
		tr->scale = optionConfig->toggleSize;
		sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->toggleLayer));
		sp->isVisible = (t > 0.0001f);
		sp->color = { optionConfig->itemColor.r, optionConfig->itemColor.g, optionConfig->itemColor.b, 0.35f * t };
	}

	if (optionToggleOnEntity_ != No::nullEntity && registry.Has<No::Transform2DComponent>(optionToggleOnEntity_) && registry.Has<No::SpriteComponent>(optionToggleOnEntity_)) {
		auto* tr = registry.GetComponent<No::Transform2DComponent>(optionToggleOnEntity_);
		auto* sp = registry.GetComponent<No::SpriteComponent>(optionToggleOnEntity_);
		const float y = basePos.y + optionConfig->itemSpacing * 3.0f;
		tr->translate = { basePos.x + optionConfig->toggleOffset.x, y + optionConfig->toggleOffset.y };
		tr->scale = optionConfig->toggleSize;
		sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->toggleLayer));
		sp->isVisible = (t > 0.0001f) && optionState->vibrationEnabled;
		sp->color = { optionConfig->toggleOnColor.r, optionConfig->toggleOnColor.g, optionConfig->toggleOnColor.b, optionConfig->toggleOnColor.a * t };
	}
	if (optionToggleOffEntity_ != No::nullEntity && registry.Has<No::Transform2DComponent>(optionToggleOffEntity_) && registry.Has<No::SpriteComponent>(optionToggleOffEntity_)) {
		auto* tr = registry.GetComponent<No::Transform2DComponent>(optionToggleOffEntity_);
		auto* sp = registry.GetComponent<No::SpriteComponent>(optionToggleOffEntity_);
		const float y = basePos.y + optionConfig->itemSpacing * 3.0f;
		tr->translate = { basePos.x + optionConfig->toggleOffset.x, y + optionConfig->toggleOffset.y };
		tr->scale = optionConfig->toggleSize;
		sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->toggleLayer));
		sp->isVisible = (t > 0.0001f) && !optionState->vibrationEnabled;
		sp->color = { optionConfig->toggleOffColor.r, optionConfig->toggleOffColor.g, optionConfig->toggleOffColor.b, optionConfig->toggleOffColor.a * t };
	}

	if (optionCursorEntity_ != No::nullEntity && registry.Has<No::Transform2DComponent>(optionCursorEntity_) && registry.Has<No::SpriteComponent>(optionCursorEntity_)) {
		auto* tr = registry.GetComponent<No::Transform2DComponent>(optionCursorEntity_);
		auto* sp = registry.GetComponent<No::SpriteComponent>(optionCursorEntity_);
		const float rowY = basePos.y + optionConfig->itemSpacing * static_cast<float>(optionState->selectedIndex);
		const No::Vector2 offset = (optionState->selectedIndex == 4) ? optionConfig->cursorBackOffset : optionConfig->cursorSelectOffset;
		tr->translate = { basePos.x + offset.x, rowY + offset.y };
		tr->scale = optionConfig->cursorSize;
		sp->layer = static_cast<uint32_t>(std::max(0, optionConfig->cursorLayer));
		sp->isVisible = (t > 0.0001f) && (optionState->phase == OptionStateComponent::OpenSelect || optionState->phase == OptionStateComponent::OpenEdit);
		const bool isEdit = (optionState->phase == OptionStateComponent::OpenEdit);
		const No::Color cc = isEdit ? optionConfig->cursorEditColor : optionConfig->cursorColor;
		sp->color = { cc.r, cc.g, cc.b, cc.a * t };
	}
}

void GameScene::UpdateOptionState()
{
	No::Registry& registry = *GetRegistry();
	OptionStateComponent* optionState = nullptr;
	OptionMenuConfigComponent* optionConfig = nullptr;
	PauseMenuConfigComponent* pauseConfig = nullptr;

	auto stateView = registry.View<CBOptionStateTag, OptionStateComponent>();
	for (auto entity : stateView) {
		optionState = registry.GetComponent<OptionStateComponent>(entity);
		if (optionState) {
			break;
		}
	}
	auto configView = registry.View<CBOptionConfigTag, OptionMenuConfigComponent>();
	for (auto entity : configView) {
		optionConfig = registry.GetComponent<OptionMenuConfigComponent>(entity);
		if (optionConfig) {
			break;
		}
	}
	auto pauseConfigView = registry.View<CBPauseConfigTag, PauseMenuConfigComponent>();
	for (auto entity : pauseConfigView) {
		pauseConfig = registry.GetComponent<PauseMenuConfigComponent>(entity);
		if (pauseConfig) {
			break;
		}
	}
	if (!optionState || !optionConfig) {
		return;
	}

	CommentBout::GameAudio::ApplyOptionVolumes(
		optionState->masterVolume,
		optionState->bgmVolume,
		optionState->seVolume
	);

	if (optionState->phase == OptionStateComponent::Closed) {
		optionState->isOpen = false;
		optionState->isEditing = false;
		optionState->isConfirmAnimating = false;
		optionState->confirmIndex = -1;
		optionState->confirmAnimTime = 0.0f;
		optionState->requestedAction = OptionStateComponent::None;
		return;
	}

	if (optionState->phase == OptionStateComponent::Opening || optionState->phase == OptionStateComponent::Closing) {
		optionState->phaseTime += GetPauseDeltaTime();
		if (optionState->phaseTime >= optionState->phaseDuration) {
			if (optionState->phase == OptionStateComponent::Opening) {
				StartOptionPhase(optionState, OptionStateComponent::OpenSelect, 1.0f);
			}
			else {
				StartOptionPhase(optionState, OptionStateComponent::Closed, 1.0f);
				optionState->isOpen = false;
				optionState->isEditing = false;
			}
		}
		return;
	}

	if (optionState->isOpen) {
	}

	if (optionState->isConfirmAnimating) {
		optionState->confirmAnimTime += GetPauseDeltaTime();
		float confirmDuration = optionConfig->confirmDuration;
		if (optionState->confirmIndex == 4 && pauseConfig) {
			confirmDuration = pauseConfig->confirmDuration;
		}
		confirmDuration = SafeDuration(confirmDuration);
		if (optionState->confirmAnimTime >= confirmDuration) {
			optionState->isConfirmAnimating = false;
			optionState->confirmAnimTime = 0.0f;
			optionState->confirmIndex = -1;

			if (optionState->requestedAction == OptionStateComponent::CloseOption) {
				StartOptionPhase(optionState, OptionStateComponent::Closing, optionConfig->closeDuration);
			}
			else if (optionState->requestedAction == OptionStateComponent::StartEdit) {
				optionState->isEditing = true;
				StartOptionPhase(optionState, OptionStateComponent::OpenEdit, 1.0f);
			}
			optionState->requestedAction = OptionStateComponent::None;
		}
		return;
	}

	if (optionState->phase == OptionStateComponent::OpenEdit) {
		const float step = std::max(0.01f, optionConfig->volumeStep);
		const bool dec = No::Keyboard::IsTrigger('A') || No::Keyboard::IsTrigger(VK_LEFT);
		const bool inc = No::Keyboard::IsTrigger('D') || No::Keyboard::IsTrigger(VK_RIGHT);
		const float beforeMaster = optionState->masterVolume;
		const float beforeBGM = optionState->bgmVolume;
		const float beforeSE = optionState->seVolume;

		if (optionState->selectedIndex == 0) {
			if (dec) optionState->masterVolume = std::max(0.0f, optionState->masterVolume - step);
			if (inc) optionState->masterVolume = std::min(1.0f, optionState->masterVolume + step);
		}
		else if (optionState->selectedIndex == 1) {
			if (dec) optionState->bgmVolume = std::max(0.0f, optionState->bgmVolume - step);
			if (inc) optionState->bgmVolume = std::min(1.0f, optionState->bgmVolume + step);
		}
		else if (optionState->selectedIndex == 2) {
			if (dec) optionState->seVolume = std::max(0.0f, optionState->seVolume - step);
			if (inc) optionState->seVolume = std::min(1.0f, optionState->seVolume + step);
		}
		else if (optionState->selectedIndex == 3) {
			if (dec || inc) {
				optionState->vibrationEnabled = !optionState->vibrationEnabled;
			}
		}

		const bool volumeChanged =
			(beforeMaster != optionState->masterVolume) ||
			(beforeBGM != optionState->bgmVolume) ||
			(beforeSE != optionState->seVolume);
		if (volumeChanged) {
			CommentBout::GameAudio::PlayTestSE();
		}

		if (No::Keyboard::IsTrigger(VK_SPACE)) {
			optionState->isEditing = false;
			StartOptionPhase(optionState, OptionStateComponent::OpenSelect, 1.0f);
		}
		return;
	}

	if (No::Keyboard::IsTrigger('W') || No::Keyboard::IsTrigger(VK_UP)) {
		optionState->selectedIndex--;
		if (optionState->selectedIndex < 0) {
			optionState->selectedIndex = optionState->itemCount - 1;
		}
	}
	if (No::Keyboard::IsTrigger('S') || No::Keyboard::IsTrigger(VK_DOWN)) {
		optionState->selectedIndex++;
		if (optionState->selectedIndex >= optionState->itemCount) {
			optionState->selectedIndex = 0;
		}
	}

	if (No::Keyboard::IsTrigger(VK_SPACE)) {
		optionState->isConfirmAnimating = true;
		optionState->confirmIndex = optionState->selectedIndex;
		optionState->confirmAnimTime = 0.0f;
		optionState->requestedAction = (optionState->selectedIndex == 4)
			? OptionStateComponent::CloseOption
			: OptionStateComponent::StartEdit;
	}
}

void GameScene::DrawAudioTestImGui()
{
#ifdef USE_IMGUI
	ImGui::Begin("AudioTest");
	ImGui::Text("Master: %.2f", CommentBout::GameAudio::GetMasterVolume());
	ImGui::Text("BGM   : %.2f (effective %.2f)", CommentBout::GameAudio::GetBGMVolume(), CommentBout::GameAudio::GetEffectiveBGMVolume());
	ImGui::Text("SE    : %.2f (effective %.2f)", CommentBout::GameAudio::GetSEVolume(), CommentBout::GameAudio::GetEffectiveSEVolume());

	if (ImGui::Button("Play Test BGM")) {
		CommentBout::GameAudio::PlayTestBGM(true);
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop Test BGM")) {
		CommentBout::GameAudio::StopTestBGM();
	}

	if (ImGui::Button("Play Test SE")) {
		CommentBout::GameAudio::PlayTestSE();
	}

	ImGui::End();
#endif
}