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
		if (No::Keyboard::IsTrigger(VK_ESCAPE) || No::Keyboard::IsTrigger(VK_RETURN)) {
			StartPhase(pauseState, PauseStateComponent::OptionClosing, pauseConfig->optionCloseDuration);
		}
		return;
	}

	if (pauseState->phase != PauseStateComponent::Open) {
		return;
	}

	if (pauseState->itemCount > 0) {
		if (No::Keyboard::IsTrigger('W')) {
			pauseState->selectedIndex--;
			if (pauseState->selectedIndex < 0) {
				pauseState->selectedIndex = pauseState->itemCount - 1;
			}
		}
		if (No::Keyboard::IsTrigger('S')) {
			pauseState->selectedIndex++;
			if (pauseState->selectedIndex >= pauseState->itemCount) {
				pauseState->selectedIndex = 0;
			}
		}
	}

	if (No::Keyboard::IsTrigger(VK_RETURN)) {
		pauseState->confirmIndex = pauseState->selectedIndex;
		pauseState->confirmAnimTime = 0.0f;
		pauseState->isConfirmAnimating = true;
		switch (pauseState->selectedIndex) {
		case 0:
			pauseState->requestedAction = PauseStateComponent::Resume;
			break;
		case 1:
			pauseState->requestedAction = PauseStateComponent::Restart;
			break;
		case 2:
			pauseState->requestedAction = PauseStateComponent::OpenOption;
			break;
		case 3:
			pauseState->requestedAction = PauseStateComponent::BackToTitle;
			break;
		default:
			pauseState->requestedAction = PauseStateComponent::None;
			pauseState->isConfirmAnimating = false;
			pauseState->confirmIndex = -1;
			pauseState->confirmAnimTime = 0.0f;
			break;
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

	if (!pauseState || !pauseConfig || pauseState->phase == PauseStateComponent::Closed) {
		return;
	}

	const float openProgress = (pauseState->phase == PauseStateComponent::Opening)
		? EaseByType(pauseConfig->easeType, pauseState->phaseTime / SafeDuration(pauseState->phaseDuration))
		: 1.0f;
	const float closeProgress = (pauseState->phase == PauseStateComponent::Closing)
		? EaseByType(pauseConfig->easeType, pauseState->phaseTime / SafeDuration(pauseState->phaseDuration))
		: 0.0f;
	const float menuVisibility = std::max(0.0f, std::min(1.0f, openProgress - closeProgress));

	ImDrawList* drawList = ImGui::GetForegroundDrawList();
	if (!drawList || menuVisibility <= 0.0001f) {
		return;
	}

	static const std::array<const char*, 4> menuItems = {
		"ゲーム再開",
		"リスタート",
		"オプション",
		"タイトルへ"
	};

	const int alphaByte = static_cast<int>(255.0f * menuVisibility);
	const ImU32 titleColor = IM_COL32(255, 255, 255, alphaByte);
	const ImU32 normalTextColor = IM_COL32(220, 220, 220, alphaByte);
	const ImU32 selectedTextColor = IM_COL32(255, 255, 120, alphaByte);
	const ImU32 selectedFrameColor = IM_COL32(255, 240, 120, alphaByte);

	const float titleShiftY = 40.0f * (1.0f - menuVisibility);
	drawList->AddText(
		nullptr,
		pauseConfig->titleSize.y,
		ImVec2(
			pauseConfig->titlePosition.x - pauseConfig->titleSize.x * 0.5f,
			pauseConfig->titlePosition.y - pauseConfig->titleSize.y * 0.5f + titleShiftY
		),
		titleColor,
		"PAUSE"
	);

	float confirmPunch = 0.0f;
	if (pauseState->isConfirmAnimating) {
		const float t = pauseState->confirmAnimTime / SafeDuration(pauseConfig->confirmDuration);
		const float eased = EaseByType(pauseConfig->easeType, t);
		confirmPunch = 1.0f - std::fabs(2.0f * eased - 1.0f);
	}

	for (int i = 0; i < pauseState->itemCount && i < static_cast<int>(menuItems.size()); ++i) {
		const float baseY = pauseConfig->itemBasePosition.y + pauseConfig->itemSpacing * static_cast<float>(i);
		const float itemShiftY = 22.0f * (1.0f - menuVisibility);
		const float y = baseY + itemShiftY;
		const ImVec2 itemCenter(pauseConfig->itemBasePosition.x, y);

		float itemScale = 1.0f;
		const bool isSelected = (i == pauseState->selectedIndex && pauseState->phase == PauseStateComponent::Open);
		if (isSelected) {
			itemScale = pauseConfig->selectedScale;
		}
		if (pauseState->isConfirmAnimating && i == pauseState->confirmIndex) {
			const float punchScale = pauseConfig->confirmScale;
			itemScale = 1.0f + (punchScale - 1.0f) * confirmPunch;
		}

		const ImVec2 halfSize(
			pauseConfig->itemSize.x * 0.5f * itemScale,
			pauseConfig->itemSize.y * 0.5f * itemScale
		);

		if (isSelected || (pauseState->isConfirmAnimating && i == pauseState->confirmIndex)) {
			drawList->AddRect(
				ImVec2(itemCenter.x - halfSize.x, itemCenter.y - halfSize.y),
				ImVec2(itemCenter.x + halfSize.x, itemCenter.y + halfSize.y),
				selectedFrameColor,
				6.0f,
				0,
				2.0f
			);
		}

		drawList->AddText(
			nullptr,
			pauseConfig->itemSize.y * 0.62f * itemScale,
			ImVec2(itemCenter.x - halfSize.x + 22.0f, itemCenter.y - pauseConfig->itemSize.y * 0.28f),
			isSelected ? selectedTextColor : normalTextColor,
			menuItems[static_cast<size_t>(i)]
		);
	}

	if (!pauseState->isConfirmAnimating && pauseState->phase == PauseStateComponent::Open) {
		const float cursorY = pauseConfig->itemBasePosition.y + pauseConfig->itemSpacing * static_cast<float>(pauseState->selectedIndex);
		drawList->AddTriangleFilled(
			ImVec2(pauseConfig->itemBasePosition.x + pauseConfig->cursorOffset.x, cursorY),
			ImVec2(pauseConfig->itemBasePosition.x + pauseConfig->cursorOffset.x + pauseConfig->cursorSize.x, cursorY - pauseConfig->cursorSize.y * 0.5f),
			ImVec2(pauseConfig->itemBasePosition.x + pauseConfig->cursorOffset.x + pauseConfig->cursorSize.x, cursorY + pauseConfig->cursorSize.y * 0.5f),
			selectedFrameColor
		);
	}

	if (pauseState->phase == PauseStateComponent::OptionOpening ||
		pauseState->phase == PauseStateComponent::OptionOpen ||
		pauseState->phase == PauseStateComponent::OptionClosing) {
		float optionT = 1.0f;
		if (pauseState->phase == PauseStateComponent::OptionOpening) {
			optionT = EaseByType(pauseConfig->easeType, pauseState->phaseTime / SafeDuration(pauseState->phaseDuration));
		} else if (pauseState->phase == PauseStateComponent::OptionClosing) {
			optionT = 1.0f - EaseByType(pauseConfig->easeType, pauseState->phaseTime / SafeDuration(pauseState->phaseDuration));
		}
		optionT = std::max(0.0f, std::min(1.0f, optionT));

		const float panelW = 560.0f;
		const float panelH = 280.0f;
		const ImVec2 center(640.0f, 360.0f);
		const ImVec2 half(panelW * 0.5f * optionT, panelH * 0.5f * optionT);

		drawList->AddRectFilled(
			ImVec2(center.x - half.x, center.y - half.y),
			ImVec2(center.x + half.x, center.y + half.y),
			IM_COL32(18, 18, 18, static_cast<int>(220.0f * optionT)),
			10.0f
		);
		drawList->AddRect(
			ImVec2(center.x - half.x, center.y - half.y),
			ImVec2(center.x + half.x, center.y + half.y),
			IM_COL32(255, 255, 180, static_cast<int>(255.0f * optionT)),
			10.0f,
			0,
			2.0f
		);

		drawList->AddText(nullptr, 42.0f * optionT, ImVec2(center.x - 120.0f, center.y - 90.0f), IM_COL32(255, 255, 255, static_cast<int>(255.0f * optionT)), "OPTION");
		drawList->AddText(nullptr, 24.0f * optionT, ImVec2(center.x - 200.0f, center.y - 20.0f), IM_COL32(220, 220, 220, static_cast<int>(255.0f * optionT)), "Enter / Esc : もどる");
		drawList->AddText(nullptr, 20.0f * optionT, ImVec2(center.x - 200.0f, center.y + 28.0f), IM_COL32(180, 180, 180, static_cast<int>(255.0f * optionT)), "(ここに音量などを追加予定)");
	}
#else
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