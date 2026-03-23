#include "stdafx.h"
#include "GameScene.h"
#include <vector>
#include <utility>
#include <array>
#include <algorithm>
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
#include "application/CommentBout/Component/RailCameraComponent.h"
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
#include "application/CommentBout/Spawner/OptionMenuSpawner.h"
#include "application/CommentBout/Spawner/PauseMenuSpawner.h"
#include "application/TestApp/System/CollisionTestSystem.h"
#include "application/TestApp/Component/Collider2DComponent.h"
#include "application/TestApp/Component/Collider3DComponent.h"
#include "application/TestApp/Component/ProjectedColliderComponent.h"
#include "engine/Runtime/GraphicsCore.h"
#include <algorithm>
#include <fstream>
#include <cstdio>
#include "externals/nlohmann/json.hpp"

namespace {
std::string MakeRailFilePath(const std::string& stageName) {
	return "resources/game/td_3105/RailData/" + stageName + "_rail.json";
}

std::string MakeEventFilePath(const std::string& stageName) {
	return "resources/game/td_3105/RailData/" + stageName + "_events.json";
}

const char* ToEventTypeString(RailEventType type) {
	switch (type) {
	case RailEventType::SpawnEnemy: return "SpawnEnemy";
	case RailEventType::RailStop: return "RailStop";
	case RailEventType::RailResume: return "RailResume";
	default: return "SpawnEnemy";
	}
}

RailEventType ParseEventType(const std::string& typeName) {
	if (typeName == "RailStop") {
		return RailEventType::RailStop;
	}
	if (typeName == "RailResume") {
		return RailEventType::RailResume;
	}
	return RailEventType::SpawnEnemy;
}

const char* ToResumeConditionString(RailResumeConditionType condition) {
	switch (condition) {
	case RailResumeConditionType::None: return "None";
	case RailResumeConditionType::AfterSeconds: return "AfterSeconds";
	case RailResumeConditionType::EnemiesCleared: return "EnemiesCleared";
	default: return "None";
	}
}

RailResumeConditionType ParseResumeCondition(const std::string& conditionName) {
	if (conditionName == "AfterSeconds") {
		return RailResumeConditionType::AfterSeconds;
	}
	if (conditionName == "EnemiesCleared") {
		return RailResumeConditionType::EnemiesCleared;
	}
	return RailResumeConditionType::None;
}

void ResetEventRuntime(RailCameraComponent& rail) {
	for (auto& eventData : rail.events) {
		eventData.fired = false;
		eventData.waitingCondition = false;
		eventData.waitingElapsedSeconds = 0.0f;
	}
}

bool SaveRailToJson(const RailCameraComponent& rail, const std::string& stageName) {
	nlohmann::json json;
	json["stageName"] = stageName;
	json["defaultSpeed"] = rail.speed;
	json["controlPoints"] = nlohmann::json::array();
	for (const auto& p : rail.controlPoints) {
		json["controlPoints"].push_back({ p.x, p.y, p.z });
	}

	std::ofstream ofs(MakeRailFilePath(stageName));
	if (!ofs) {
		return false;
	}
	ofs << json.dump(2);
	return true;
}

bool SaveEventsToJson(const RailCameraComponent& rail, const std::string& stageName) {
	nlohmann::json json;
	json["stageName"] = stageName;
	json["events"] = nlohmann::json::array();

	for (const auto& e : rail.events) {
		nlohmann::json eventJson;
		eventJson["type"] = ToEventTypeString(e.type);
		eventJson["triggerDistance"] = e.triggerDistance;
		eventJson["resumeCondition"] = ToResumeConditionString(e.resumeCondition);
		eventJson["resumeAfterSeconds"] = e.resumeAfterSeconds;
		eventJson["targetGroupId"] = e.targetGroupId;
		eventJson["spawn"] = {
			{ "count", e.spawn.count },
			{ "hp", e.spawn.hp },
			{ "moveSpeed", e.spawn.moveSpeed },
			{ "spawnSpacing", e.spawn.spawnSpacing },
			{ "spawnPosition", { e.spawn.spawnPosition.x, e.spawn.spawnPosition.y, e.spawn.spawnPosition.z } },
			{ "moveDirection", { e.spawn.moveDirection.x, e.spawn.moveDirection.y, e.spawn.moveDirection.z } },
			{ "spawnGroupId", e.spawn.spawnGroupId }
		};
		json["events"].push_back(eventJson);
	}

	std::ofstream ofs(MakeEventFilePath(stageName));
	if (!ofs) {
		return false;
	}
	ofs << json.dump(2);
	return true;
}

bool LoadRailToComponent(RailCameraComponent& rail, const std::string& stageName) {
	const std::string filePath = MakeRailFilePath(stageName);
	std::ifstream ifs(filePath);
	if (!ifs) {
		return false;
	}

	nlohmann::json json;
	ifs >> json;

	auto pointsIt = json.find("controlPoints");
	if (pointsIt == json.end() || !pointsIt->is_array()) {
		return false;
	}

	rail.controlPoints.clear();
	for (const auto& p : *pointsIt) {
		if (!p.is_array() || p.size() < 3) {
			continue;
		}
		No::Vector3 point{};
		point.x = p[0].get<float>();
		point.y = p[1].get<float>();
		point.z = p[2].get<float>();
		rail.controlPoints.push_back(point);
	}

	if (rail.controlPoints.size() < 2) {
		return false;
	}

	const auto speedIt = json.find("defaultSpeed");
	if (speedIt != json.end() && speedIt->is_number()) {
		rail.speed = speedIt->get<float>();
	}

	rail.stageName = stageName;
	rail.railFilePath = filePath;
	rail.selectedControlPointIndex = (rail.controlPoints.empty()) ? -1 : 0;
	rail.distance = 0.0f;
	rail.isFinished = false;
	rail.isPlaying = true;
	rail.isLoaded = false;
	rail.needsRebuildArcLength = true;
	ResetEventRuntime(rail);
	return true;
}

bool LoadEventsToComponent(RailCameraComponent& rail, const std::string& stageName) {
	const std::string filePath = MakeEventFilePath(stageName);
	std::ifstream ifs(filePath);
	if (!ifs) {
		return false;
	}

	nlohmann::json json;
	ifs >> json;

	auto eventsIt = json.find("events");
	if (eventsIt == json.end() || !eventsIt->is_array()) {
		return false;
	}

	rail.events.clear();
	for (const auto& eventJson : *eventsIt) {
		RailEventData eventData;

		const auto typeIt = eventJson.find("type");
		if (typeIt != eventJson.end() && typeIt->is_string()) {
			eventData.type = ParseEventType(typeIt->get<std::string>());
		}
		const auto triggerIt = eventJson.find("triggerDistance");
		if (triggerIt != eventJson.end() && triggerIt->is_number()) {
			eventData.triggerDistance = triggerIt->get<float>();
		}

		const auto conditionIt = eventJson.find("resumeCondition");
		if (conditionIt != eventJson.end() && conditionIt->is_string()) {
			eventData.resumeCondition = ParseResumeCondition(conditionIt->get<std::string>());
		}
		const auto afterIt = eventJson.find("resumeAfterSeconds");
		if (afterIt != eventJson.end() && afterIt->is_number()) {
			eventData.resumeAfterSeconds = afterIt->get<float>();
		}
		const auto targetGroupIt = eventJson.find("targetGroupId");
		if (targetGroupIt != eventJson.end() && targetGroupIt->is_number_integer()) {
			eventData.targetGroupId = targetGroupIt->get<int>();
		}

		const auto spawnIt = eventJson.find("spawn");
		if (spawnIt != eventJson.end() && spawnIt->is_object()) {
			const auto countIt = spawnIt->find("count");
			if (countIt != spawnIt->end() && countIt->is_number_integer()) {
				eventData.spawn.count = countIt->get<int>();
			}
			const auto hpIt = spawnIt->find("hp");
			if (hpIt != spawnIt->end() && hpIt->is_number_integer()) {
				eventData.spawn.hp = hpIt->get<int>();
			}
			const auto speedIt = spawnIt->find("moveSpeed");
			if (speedIt != spawnIt->end() && speedIt->is_number()) {
				eventData.spawn.moveSpeed = speedIt->get<float>();
			}
			const auto spacingIt = spawnIt->find("spawnSpacing");
			if (spacingIt != spawnIt->end() && spacingIt->is_number()) {
				eventData.spawn.spawnSpacing = spacingIt->get<float>();
			}
			const auto spawnPosIt = spawnIt->find("spawnPosition");
			if (spawnPosIt != spawnIt->end() && spawnPosIt->is_array() && spawnPosIt->size() >= 3) {
				eventData.spawn.spawnPosition.x = (*spawnPosIt)[0].get<float>();
				eventData.spawn.spawnPosition.y = (*spawnPosIt)[1].get<float>();
				eventData.spawn.spawnPosition.z = (*spawnPosIt)[2].get<float>();
			}
			const auto moveDirIt = spawnIt->find("moveDirection");
			if (moveDirIt != spawnIt->end() && moveDirIt->is_array() && moveDirIt->size() >= 3) {
				eventData.spawn.moveDirection.x = (*moveDirIt)[0].get<float>();
				eventData.spawn.moveDirection.y = (*moveDirIt)[1].get<float>();
				eventData.spawn.moveDirection.z = (*moveDirIt)[2].get<float>();
			}
			const auto spawnGroupIt = spawnIt->find("spawnGroupId");
			if (spawnGroupIt != spawnIt->end() && spawnGroupIt->is_number_integer()) {
				eventData.spawn.spawnGroupId = spawnGroupIt->get<int>();
			}
		}

		rail.events.push_back(eventData);
	}

	rail.selectedEventIndex = rail.events.empty() ? -1 : 0;
	ResetEventRuntime(rail);
	return true;
}
}

void GameScene::Setup() {

	grassNameIndex_ = 0;

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
#ifdef USE_IMGUI
	if (railCameraEntity_ == No::nullEntity) {
		return;
	}

	auto* rail = GetRegistry()->GetComponent<RailCameraComponent>(railCameraEntity_);
	if (!rail) {
		return;
	}

	ImGui::Begin("RailCamera");
	ImGui::Text("Rail File: %s", rail->railFilePath.c_str());
	ImGui::Text("Loaded: %s", rail->isLoaded ? "Yes" : "No");
	ImGui::Text("Control Points: %d", static_cast<int>(rail->controlPoints.size()));
	ImGui::Text("Events: %d", static_cast<int>(rail->events.size()));
	int aliveRailEnemyCount = 0;
	for (auto e : GetRegistry()->View<CBRailEnemyTag, RailEnemyComponent>()) {
		(void)e;
		++aliveRailEnemyCount;
	}
	ImGui::Text("Alive Rail Enemies: %d", aliveRailEnemyCount);

	if (rail->totalLength > 0.0f) {
		const float progress = rail->distance / rail->totalLength;
		ImGui::Text("Progress: %.1f%%", progress * 100.0f);
		ImGui::Text("Distance: %.2f / %.2f", rail->distance, rail->totalLength);
	} else {
		ImGui::Text("Progress: 0.0%%");
		ImGui::Text("Distance: %.2f / %.2f", rail->distance, rail->totalLength);
	}

	if (ImGui::SliderFloat("Speed", &rail->speed, 0.0f, 40.0f)) {
		if (rail->speed < 0.0f) {
			rail->speed = 0.0f;
		}
	}
	ImGui::Checkbox("Draw Rail", &rail->drawRailDebug);
	ImGui::Checkbox("Draw Camera Gizmo", &rail->drawCameraDebug);
	ImGui::Checkbox("Draw Control Points", &rail->drawControlPointsDebug);
	ImGui::DragFloat("Control Point Radius", &rail->controlPointDebugRadius, 0.005f, 0.01f, 1.0f);

	if (ImGui::Button(rail->isPlaying ? "Pause" : "Play")) {
		if (!rail->isFinished || rail->distance < rail->totalLength) {
			rail->isPlaying = !rail->isPlaying;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop")) {
		rail->distance = 0.0f;
		rail->isPlaying = false;
		rail->isFinished = false;
	}
	ImGui::SameLine();
	if (ImGui::Button("ResetAndPlay")) {
		rail->distance = 0.0f;
		rail->isFinished = false;
		rail->isPlaying = true;
		ResetEventRuntime(*rail);
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset Event Runtime")) {
		ResetEventRuntime(*rail);
	}

	float percent = 0.0f;
	if (rail->totalLength > 0.0f) {
		percent = (rail->distance / rail->totalLength) * 100.0f;
	}
	if (ImGui::SliderFloat("Progress(%)", &percent, 0.0f, 100.0f)) {
		if (rail->totalLength > 0.0f) {
			rail->distance = rail->totalLength * (percent / 100.0f);
			rail->isFinished = (rail->distance >= rail->totalLength);
			if (rail->isFinished) {
				rail->isPlaying = false;
			}
		}
	}

	if (rail->isFinished) {
		ImGui::Text("State: Finished");
	} else if (rail->isPlaying) {
		ImGui::Text("State: Playing");
	} else {
		ImGui::Text("State: Paused");
	}

	ImGui::End();
#endif
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

	ImGui::Begin("RailEditor");
	ImGui::InputText("StageName", stageNameBuffer, sizeof(stageNameBuffer));

	if (ImGui::Button("Load Stage Rail")) {
		const std::string stageName(stageNameBuffer);
		if (!stageName.empty()) {
			LoadRailToComponent(*rail, stageName);
			LoadEventsToComponent(*rail, stageName);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Save Stage Rail")) {
		const std::string stageName(stageNameBuffer);
		if (!stageName.empty()) {
			rail->stageName = stageName;
			rail->railFilePath = MakeRailFilePath(stageName);
			SaveRailToJson(*rail, stageName);
			SaveEventsToJson(*rail, stageName);
		}
	}

	ImGui::Separator();
	if (ImGui::Button("Add Control Point")) {
		No::Vector3 newPoint = { 0.0f, 0.0f, 0.0f };
		if (!rail->controlPoints.empty()) {
			newPoint = rail->controlPoints.back();
			newPoint.z += 2.0f;
		}
		rail->controlPoints.push_back(newPoint);
		rail->selectedControlPointIndex = static_cast<int>(rail->controlPoints.size()) - 1;
		rail->isLoaded = false;
		rail->needsRebuildArcLength = true;
	}

	if (ImGui::Button("Delete Selected Point")) {
		const int index = rail->selectedControlPointIndex;
		if (index >= 0 && index < static_cast<int>(rail->controlPoints.size())) {
			rail->controlPoints.erase(rail->controlPoints.begin() + index);
			if (rail->controlPoints.empty()) {
				rail->selectedControlPointIndex = -1;
			} else if (rail->selectedControlPointIndex >= static_cast<int>(rail->controlPoints.size())) {
				rail->selectedControlPointIndex = static_cast<int>(rail->controlPoints.size()) - 1;
			}
			rail->isLoaded = false;
			rail->needsRebuildArcLength = true;
		}
	}

	if (!rail->controlPoints.empty()) {
		ImGui::Separator();
		for (int i = 0; i < static_cast<int>(rail->controlPoints.size()); ++i) {
			char label[32];
			std::snprintf(label, sizeof(label), "Point %d", i);
			if (ImGui::Selectable(label, rail->selectedControlPointIndex == i)) {
				rail->selectedControlPointIndex = i;
			}
		}

		const int selected = rail->selectedControlPointIndex;
		if (selected >= 0 && selected < static_cast<int>(rail->controlPoints.size())) {
			No::Vector3& p = rail->controlPoints[static_cast<size_t>(selected)];
			if (ImGui::DragFloat3("Selected Position", &p.x, 0.05f)) {
				rail->isLoaded = false;
				rail->needsRebuildArcLength = true;
			}
		}
	}

	ImGui::Separator();
	ImGui::Text("Stage Events");
	if (ImGui::Button("Add SpawnEnemy Event")) {
		RailEventData newEvent;
		newEvent.type = RailEventType::SpawnEnemy;
		newEvent.triggerDistance = rail->distance;
		rail->events.push_back(newEvent);
		rail->selectedEventIndex = static_cast<int>(rail->events.size()) - 1;
	}
	ImGui::SameLine();
	if (ImGui::Button("Add Stop Event")) {
		RailEventData newEvent;
		newEvent.type = RailEventType::RailStop;
		newEvent.triggerDistance = rail->distance;
		rail->events.push_back(newEvent);
		rail->selectedEventIndex = static_cast<int>(rail->events.size()) - 1;
	}
	ImGui::SameLine();
	if (ImGui::Button("Add Resume Event")) {
		RailEventData newEvent;
		newEvent.type = RailEventType::RailResume;
		newEvent.triggerDistance = rail->distance;
		newEvent.resumeCondition = RailResumeConditionType::AfterSeconds;
		newEvent.resumeAfterSeconds = 1.0f;
		rail->events.push_back(newEvent);
		rail->selectedEventIndex = static_cast<int>(rail->events.size()) - 1;
	}

	if (ImGui::Button("Delete Selected Event")) {
		const int selected = rail->selectedEventIndex;
		if (selected >= 0 && selected < static_cast<int>(rail->events.size())) {
			rail->events.erase(rail->events.begin() + selected);
			if (rail->events.empty()) {
				rail->selectedEventIndex = -1;
			} else if (rail->selectedEventIndex >= static_cast<int>(rail->events.size())) {
				rail->selectedEventIndex = static_cast<int>(rail->events.size()) - 1;
			}
		}
	}

	for (int i = 0; i < static_cast<int>(rail->events.size()); ++i) {
		const RailEventData& e = rail->events[static_cast<size_t>(i)];
		char label[160];
		if (e.type == RailEventType::SpawnEnemy) {
			std::snprintf(label, sizeof(label), "Event %d : %s [G%d] @ %.2f", i, ToEventTypeString(e.type), e.spawn.spawnGroupId, e.triggerDistance);
		} else if (e.type == RailEventType::RailResume && e.resumeCondition == RailResumeConditionType::EnemiesCleared) {
			std::snprintf(label, sizeof(label), "Event %d : %s [TargetG%d] @ %.2f", i, ToEventTypeString(e.type), e.targetGroupId, e.triggerDistance);
		} else {
			std::snprintf(label, sizeof(label), "Event %d : %s @ %.2f", i, ToEventTypeString(e.type), e.triggerDistance);
		}
		if (ImGui::Selectable(label, rail->selectedEventIndex == i)) {
			rail->selectedEventIndex = i;
		}
	}

	if (rail->selectedEventIndex >= 0 && rail->selectedEventIndex < static_cast<int>(rail->events.size())) {
		RailEventData& e = rail->events[static_cast<size_t>(rail->selectedEventIndex)];
		ImGui::Separator();
		int eventType = static_cast<int>(e.type);
		if (ImGui::Combo("EventType", &eventType, "SpawnEnemy\0RailStop\0RailResume\0")) {
			e.type = static_cast<RailEventType>(eventType);
		}
		ImGui::DragFloat("TriggerDistance", &e.triggerDistance, 0.1f, 0.0f, (rail->totalLength > 0.0f) ? rail->totalLength : 10000.0f);

		if (e.type == RailEventType::SpawnEnemy) {
			ImGui::DragInt("Spawn Count", &e.spawn.count, 1.0f, 1, 32);
			ImGui::DragInt("Enemy HP", &e.spawn.hp, 1.0f, 1, 100);
			ImGui::DragFloat("Enemy Speed", &e.spawn.moveSpeed, 0.1f, 0.0f, 100.0f);
			ImGui::DragFloat("Spawn Spacing", &e.spawn.spawnSpacing, 0.1f, 0.0f, 20.0f);
			ImGui::DragFloat3("Spawn Position", &e.spawn.spawnPosition.x, 0.1f);
			ImGui::DragFloat3("Move Direction", &e.spawn.moveDirection.x, 0.05f);
			ImGui::DragInt("Spawn GroupId", &e.spawn.spawnGroupId, 1.0f, 0, 999);
		}
		if (e.type == RailEventType::RailResume) {
			int conditionType = static_cast<int>(e.resumeCondition);
			if (ImGui::Combo("ResumeCondition", &conditionType, "None\0AfterSeconds\0EnemiesCleared\0")) {
				e.resumeCondition = static_cast<RailResumeConditionType>(conditionType);
			}
			if (e.resumeCondition == RailResumeConditionType::AfterSeconds) {
				ImGui::DragFloat("ResumeAfterSeconds", &e.resumeAfterSeconds, 0.05f, 0.0f, 120.0f);
			}
			if (e.resumeCondition == RailResumeConditionType::EnemiesCleared) {
				ImGui::DragInt("Target GroupId", &e.targetGroupId, 1.0f, 0, 999);
			}
		}

		ImGui::Text("Runtime: fired=%s waiting=%s elapsed=%.2f", e.fired ? "true" : "false", e.waitingCondition ? "true" : "false", e.waitingElapsedSeconds);
	}

	ImGui::End();
#endif
}
