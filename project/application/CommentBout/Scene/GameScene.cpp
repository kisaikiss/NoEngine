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
#include "application/CommentBout/Component/GrassReactionComponent.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/Component/GroundComponent.h"
#include "application/CommentBout/Utility/CBCollisionMask.h"
#include "application/CommentBout/Utility/CBSpriteLayer.h"
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

	// ---- システム登録（順序が重要）----------------------------------------------
	// 1. PlayerControlSystem    : 入力処理・攻撃エンティティのスポーン
	// 2. CollisionTestSystem    : コライダー更新・投影・衝突判定
	//                             → projected->isColliding をセット
	// 3. GrassReactionSystem    : 衝突開始を検出しエフェクトエンティティをスポーン
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

	auto* groundCollider = registry.AddComponent<TestApp::Collider3DComponent>(groundEntity);
	groundCollider->shapeType = TestApp::ShapeType3D::Box;
	groundCollider->useScaleAsBox = true;
	groundCollider->boxSizeMultiplier = { 1.f, 1.f, 1.f };
	groundCollider->collisionLayer = CommentBout::CollisionLayer::CBGround;
	groundCollider->collisionMask = CommentBout::CollisionMask::CBGround;

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
}

void GameScene::UpdatePauseState()
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
				StartPhase(pauseState, PauseStateComponent::OptionOpening, pauseConfig->optionOpenDuration);
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
		pauseState->phase == PauseStateComponent::Closing ||
		pauseState->phase == PauseStateComponent::OptionOpening ||
		pauseState->phase == PauseStateComponent::OptionClosing) {
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
			case PauseStateComponent::OptionOpening:
				StartPhase(pauseState, PauseStateComponent::OptionOpen, 1.0f);
				pauseState->phaseTime = 0.0f;
				break;
			case PauseStateComponent::OptionClosing:
				StartPhase(pauseState, PauseStateComponent::Open, 1.0f);
				pauseState->phaseTime = 0.0f;
				break;
			default:
				break;
			}
		}
		return;
	}

	if (pauseState->isConfirmAnimating) {
		return;
	}

	if (pauseState->phase == PauseStateComponent::OptionOpen) {
		if (No::Keyboard::IsTrigger(VK_RETURN)) {
			StartPhase(pauseState, PauseStateComponent::OptionClosing, pauseConfig->optionCloseDuration);
		}
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
			action = PauseStateComponent::None;
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
	lineSprite->layer = CommentBout::ToLayer(CommentBout::SpriteLayer::PausePanelLine);
	auto* lineTag = registry.AddComponent<No::EditTag>(pausePanelLineEntity_);
	lineTag->name = "PausePanelLineSprite";

	pauseTitleEntity_ = registry.GenerateEntity();
	auto* titleTransform = registry.AddComponent<No::Transform2DComponent>(pauseTitleEntity_);
	titleTransform->translate = { 640.0f, 220.0f };
	titleTransform->scale = { 480.0f, 120.0f };
	auto* titleSprite = registry.AddComponent<No::SpriteComponent>(pauseTitleEntity_);
	titleSprite->textureHandle = pauseTitleTexture;
	titleSprite->isVisible = false;
	titleSprite->color = { 1.0f, 1.0f, 1.0f, 0.0f };
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
		itemSprite->color = { 1.0f, 1.0f, 1.0f, 0.0f };
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