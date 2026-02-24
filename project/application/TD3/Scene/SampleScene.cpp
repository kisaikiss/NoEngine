#include "SampleScene.h"
#include "../Component/GridCellComponent.h"
#include "../Component/PlayerComponent.h"
#include "../Component/EnemyComponent.h"
#include "../Component/HealthComponent.h"
#include "../Component/ColliderComponent.h"
#include "../GameTag.h"
#include "../System/GridRenderSystem.h"
#include "../System/PlayerMovementSystem.h"
#include "../System/PlayerWeaponSystem.h"
#include "../System/PlayerBulletSystem.h"
#include "../System/AmmoItemSystem.h"
#include "../System/EnemyMovementSystem.h"
#include "../System/CollisionSystem.h"
#include "../System/EnemyCollisionSystem.h"
#include "../MapData/MapLoader.h"
#include <vector>
#include <string>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

void SampleScene::Setup() {
	AddSystem(std::make_unique<GridRenderSystem>());
	AddSystem(std::make_unique<PlayerMovementSystem>());
	AddSystem(std::make_unique<EnemyMovementSystem>());
	AddSystem(std::make_unique<PlayerBulletSystem>());
	AddSystem(std::make_unique<CollisionSystem>());
	AddSystem(std::make_unique<EnemyCollisionSystem>());
	AddSystem(std::make_unique<PlayerWeaponSystem>());
	AddSystem(std::make_unique<AmmoItemSystem>());

	No::Registry& registry = *GetRegistry();

	stageNumber_ = 1;
	std::string path = "resources/game/td_3105/Stages/stage_0"
		+ std::to_string(stageNumber_) + ".json";
	MapData::StageData stageData = MapLoader::LoadStage(path);

	InitializeGrid(registry, stageData.connectionMap);
	for (const auto& entity : stageData.entityMap.entities) {
		if (entity.type == "player")     InitializePlayer(registry, entity.x, entity.y);
		else if (entity.type == "enemy") InitializeEnemy(registry, entity.x, entity.y);
	}

	InitializeLight(registry);

	camera_ = std::make_unique<NoEngine::Camera>();
	cameraTransform_.translate = { 1.0f, 1.0f, -15.0f };
	camera_->SetTransform(cameraTransform_);
	SetCamera(camera_.get());
}

// ============================================================
//  ReloadStage
//  ECS を全リセットし、指定ステージを再ロードする。
//  システムは再登録しない（AddSystem は Setup で1回だけでよい）。
// ============================================================

void SampleScene::ReloadStage(int stageNumber) {
	stageNumber_ = stageNumber;
	No::Registry& registry = *GetRegistry();

	// ---- 全エンティティを View で収集してからまとめて削除 ----
	std::vector<No::Entity> all;
	for (auto e : registry.View<GridCellComponent>())     all.push_back(e);
	for (auto e : registry.View<PlayerComponent>())       all.push_back(e);
	for (auto e : registry.View<EnemyComponent>())        all.push_back(e);
	for (auto e : registry.View<PlayerBulletComponent>()) all.push_back(e);
	for (auto e : registry.View<AmmoItemComponent>())     all.push_back(e);

	for (auto e : all) {
		registry.DestroyEntity(e);
	}

	// ---- 指定ステージを読み込んで再初期化 ----
	std::string path = "resources/game/td_3105/Stages/stage_0"
		+ std::to_string(stageNumber_) + ".json";
	MapData::StageData stageData = MapLoader::LoadStage(path);

	InitializeGrid(registry, stageData.connectionMap);
	for (const auto& entity : stageData.entityMap.entities) {
		if (entity.type == "player")     InitializePlayer(registry, entity.x, entity.y);
		else if (entity.type == "enemy") InitializeEnemy(registry, entity.x, entity.y);
	}
}

// ============================================================
//  UpdateGame
//  クリア・ゲームオーバーを検出する。
//  NotSystemUpdate から毎フレーム呼ばれる。
// ============================================================

void SampleScene::UpdateGame(No::Registry& registry) {
	// ---- ゲームオーバー判定 ----
	auto playerView = registry.View<PlayerTag, DeathFlag>();
	for (auto entity : playerView) {
		auto* flag = registry.GetComponent<DeathFlag>(entity);
		if (flag && flag->isDead) {
			ReloadStage(stageNumber_);
			return;
		}
	}

	// ---- クリア判定（敵が全滅したとき） ----
	auto enemyView = registry.View<EnemyTag, DeathFlag>();
	if (enemyView.Empty()) return;

	int aliveEnemies = 0;
	for (auto entity : enemyView) {
		auto* flag = registry.GetComponent<DeathFlag>(entity);
		if (flag && !flag->isDead) aliveEnemies++;
	}

	if (aliveEnemies == 0) {
		// 次ステージへ。JSON が存在しない場合は MapLoader が例外を投げる。
		// 将来はステージ数の上限チェックをここに追加する。
		ReloadStage(stageNumber_ + 1);
	}
}

// ============================================================
//  DebugStageControlUI
// ============================================================

#ifdef USE_IMGUI
void SampleScene::DebugStageControlUI(No::Registry& registry) {

	(void)registry;

	ImGui::Begin("Stage Control");

	ImGui::Text("Current Stage: %d", stageNumber_);
	ImGui::Separator();

	// ステージ番号を指定してロード
	static int inputStageNumber = 1;
	ImGui::InputInt("Stage Number", &inputStageNumber);
	if (inputStageNumber < 1) inputStageNumber = 1;

	if (ImGui::Button("Load Stage")) {
		ReloadStage(inputStageNumber);
	}
	ImGui::SameLine();
	if (ImGui::Button("Reload Current")) {
		ReloadStage(stageNumber_);
	}

	ImGui::Separator();

	// エディタモード切り替え（Stage7 実装まで無効化）
	ImGui::BeginDisabled();
	ImGui::Checkbox("Editor Mode", &isEditorMode_);
	ImGui::EndDisabled();
	ImGui::TextDisabled("(Editor: Stage7 で実装予定)");

	ImGui::End();
}
#endif

// ============================================================
//  NotSystemUpdate
// ============================================================

void SampleScene::NotSystemUpdate() {
	No::Registry& registry = *GetRegistry();

#ifdef USE_IMGUI
	ImGui::Begin("camera");
	ImGui::DragFloat3("pos", &cameraTransform_.translate.x, 0.1f);
	ImGui::DragFloat3("rot", &cameraTransform_.rotation.x, 0.1f);
	ImGui::End();
	camera_->SetTransform(cameraTransform_);

	DebugStageControlUI(registry);
#endif

	camera_->Update();

	// エディタモード中はゲーム進行チェックをスキップ（Stage7 対応）
	if (!isEditorMode_) {
		UpdateGame(registry);
	}

	DestroyGameObject();
}

// ============================================================
//  InitializeGrid
// ============================================================

void SampleScene::InitializeGrid(No::Registry& registry, const MapData::ConnectionMapData& mapData) {
	for (const auto& nodeData : mapData.nodes) {
		auto  entity = registry.GenerateEntity();
		auto* cell = registry.AddComponent<GridCellComponent>(entity);
		cell->gridX = nodeData.x;
		cell->gridY = nodeData.y;
		cell->hasConnectionUp = nodeData.up;
		cell->hasConnectionRight = nodeData.right;
		cell->hasConnectionDown = nodeData.down;
		cell->hasConnectionLeft = nodeData.left;
	}
}

// ============================================================
//  InitializePlayer
// ============================================================

void SampleScene::InitializePlayer(No::Registry& registry, int startX, int startY) {
	auto  entity = registry.GenerateEntity();
	auto* player = registry.AddComponent<PlayerComponent>(entity);
	player->currentNodeX = startX;
	player->currentNodeY = startY;
	player->targetNodeX = startX;
	player->targetNodeY = startY;

	registry.AddComponent<PlayerTag>(entity);
	registry.AddComponent<DeathFlag>(entity);

	auto* health = registry.AddComponent<HealthComponent>(entity);
	int HP = 3;
	health->currentHp = HP;
	health->maxHp = HP;

	auto* collider = registry.AddComponent<SphereColliderComponent>(entity);
	collider->radius = 0.5f;
	collider->colliderType = kPlayer;
	collider->collideMask = kEnemy;

	auto* transform = registry.AddComponent<No::TransformComponent>(entity);
	transform->scale = { 0.2f, 0.2f, 0.2f };

	auto* mesh = registry.AddComponent<No::MeshComponent>(entity);
	auto* material = registry.AddComponent<No::MaterialComponent>(entity);
	NoEngine::Asset::ModelLoader::LoadModel(
		"TestPlayer",
		"resources/game/td_3105/Model/TestPlayer/TestPlayer.obj",
		mesh
	);
	material->materials = NoEngine::Asset::ModelLoader::GetMaterial("TestPlayer");
	material->psoName = L"Renderer : Default PSO";
	material->psoId = NoEngine::Render::GetPSOID(material->psoName);
	material->rootSigId = NoEngine::Render::GetRootSignatureID(material->psoName);
}

// ============================================================
//  InitializeEnemy
// ============================================================

void SampleScene::InitializeEnemy(No::Registry& registry, int startX, int startY) {
	auto  entity = registry.GenerateEntity();
	auto* enemy = registry.AddComponent<EnemyComponent>(entity);
	enemy->currentNodeX = startX;
	enemy->currentNodeY = startY;
	enemy->targetNodeX = startX;
	enemy->targetNodeY = startY;

	registry.AddComponent<EnemyTag>(entity);
	registry.AddComponent<DeathFlag>(entity);
	registry.AddComponent<HealthComponent>(entity);

	auto* collider = registry.AddComponent<SphereColliderComponent>(entity);
	collider->radius = 0.5f;
	collider->colliderType = kEnemy;
	collider->collideMask = kPlayer | kPlayerBullet;

	auto* transform = registry.AddComponent<No::TransformComponent>(entity);
	transform->scale = { 0.2f, 0.2f, 0.2f };

	auto* mesh = registry.AddComponent<No::MeshComponent>(entity);
	auto* material = registry.AddComponent<No::MaterialComponent>(entity);
	NoEngine::Asset::ModelLoader::LoadModel(
		"Enemy",
		"resources/game/td_3105/Model/TestPlayer/TestPlayer.obj",
		mesh
	);
	material->materials = NoEngine::Asset::ModelLoader::GetMaterial("Enemy");
	material->color = { 1.0f, 0.2f, 0.2f, 1.0f };
	material->psoName = L"Renderer : Default PSO";
	material->psoId = NoEngine::Render::GetPSOID(material->psoName);
	material->rootSigId = NoEngine::Render::GetRootSignatureID(material->psoName);
}

// ============================================================
//  InitializeLight
// ============================================================

void SampleScene::InitializeLight(No::Registry& registry) {
	auto  lightEntity = registry.GenerateEntity();
	auto* light = registry.AddComponent<NoEngine::Component::DirectionalLightComponent>(lightEntity);
	light->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	light->direction = { 0.0f, -1.0f, 0.0f };
	light->intensity = 1.0f;
}

// ============================================================
//  DestroyGameObject
// ============================================================

void SampleScene::DestroyGameObject() {
	No::Registry& registry = *GetRegistry();
	if (registry.View<DeathFlag>().Empty()) return;

	std::vector<No::Entity> toDestroy;
	auto view = registry.View<DeathFlag>();
	for (auto entity : view) {
		if (registry.Has<DeathFlag>(entity)) {
			auto* flag = registry.GetComponent<DeathFlag>(entity);
			if (flag && flag->isDead) toDestroy.push_back(entity);
		}
	}
	for (auto entity : toDestroy) {
		registry.DestroyEntity(entity);
	}
}