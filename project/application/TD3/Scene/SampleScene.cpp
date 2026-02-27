#include "SampleScene.h"
#include "../Component/GridCellComponent.h"
#include "../Component/PlayerComponent.h"
#include "../Component/EnemyComponent.h"
#include "../Component/HealthComponent.h"
#include "../Component/ColliderComponent.h"
#include "../Component/PlayerBulletComponent.h"
#include "../Component/AmmoItemComponent.h"
#include "../GameTag.h"
#include "../System/GridRenderSystem.h"
#include "../System/PlayerMovementSystem.h"
#include "../System/PlayerWeaponSystem.h"
#include "../System/PlayerBulletSystem.h"
#include "../System/AmmoItemSystem.h"
#include "../System/EnemyMovementSystem.h"
#include "../System/CollisionSystem.h"
#include "../System/EnemyCollisionSystem.h"
#include "../System/EnemyToEnemyCollisionSystem.h"   // 追加
#include "../MapData/MapLoader.h"
#include "../Utility/GridUtils.h"
#include <vector>
#include <string>
#include <algorithm>   // std::min, std::max

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
	AddSystem(std::make_unique<EnemyToEnemyCollisionSystem>());  // 追加
	AddSystem(std::make_unique<PlayerWeaponSystem>());
	AddSystem(std::make_unique<AmmoItemSystem>());

	No::Registry& registry = *GetRegistry();

#ifdef USE_IMGUI
	editor_ = std::make_unique<MapEditor>();
#endif

	stageNumber_ = 1;
	std::string path = "resources/game/td_3105/Stages/stage_0"
		+ std::to_string(stageNumber_) + ".json";
	MapData::StageData stageData = MapLoader::LoadStage(path);

	GridUtils::gGridScale = stageData.connectionMap.gridScale;

	InitializeGrid(registry, stageData.connectionMap);
	for (const auto& entity : stageData.entityMap.entities) {
		if (entity.type == "player")     InitializePlayer(registry, entity.x, entity.y);
		else if (entity.type == "enemy") InitializeEnemy(registry, entity.x, entity.y);
	}

	InitializeLight(registry);

	camera_ = std::make_unique<NoEngine::Camera>();
	SetupCameraForStage(stageData.connectionMap);   // 自動配置
	SetCamera(camera_.get());
}

// ============================================================
//  SetupCameraForStage
//  全ノードのXY座標からマップ中心とZ距離を自動計算する。
// ============================================================

void SampleScene::SetupCameraForStage(const MapData::ConnectionMapData& mapData) {
	if (mapData.nodes.empty()) {
		// ノードがない場合はデフォルト値
		cameraTransform_.translate = { 0.0f, 0.0f, -15.0f };
		camera_->SetTransform(cameraTransform_);
		return;
	}

	// 全ノードのmin/maxを求める
	int minX = mapData.nodes[0].x;
	int maxX = mapData.nodes[0].x;
	int minY = mapData.nodes[0].y;
	int maxY = mapData.nodes[0].y;

	for (const auto& node : mapData.nodes) {
		minX = std::min(minX, node.x);
		maxX = std::max(maxX, node.x);
		minY = std::min(minY, node.y);
		maxY = std::max(maxY, node.y);
	}

	// マップ中心（ワールド座標）
	float centerX = (static_cast<float>(minX + maxX) * 0.5f) * GridUtils::gGridScale;
	float centerY = (static_cast<float>(minY + maxY) * 0.5f) * GridUtils::gGridScale;

	// マップの広さ（グリッド数 × スケール）
	float spanX = static_cast<float>(maxX - minX) * GridUtils::gGridScale;
	float spanY = static_cast<float>(maxY - minY) * GridUtils::gGridScale;

	// Camera::fovY_ = 0.45f（Camera.cppのコンストラクタで固定されている値）
	// アスペクト比は取得できないため、X/Y両方向で必要なZを計算して大きい方を採用する。
	//
	// 【計算式】
	//   画角の半分: halfFov = fovY / 2
	//   Zから見えるY方向の半分: visibleHalf = Z * tan(halfFov)
	//   spanY を収めるには: Z >= (spanY/2) / tan(halfFov)
	//
	// アスペクト比が不明なので spanX / spanY の比率でアスペクトを推定し
	// 横方向の必要Zも計算して安全側（大きい方）を使う。
	const float FOV_Y = 0.45f;             // Camera.cppと合わせる
	const float MARGIN = 1.3f;             // 余白係数（大きいほど余裕が出る）
	const float Z_MIN = 5.0f;             // 最低Z距離

	float halfFov = FOV_Y * 0.5f;
	float tanHalfFov = std::tan(halfFov);    // ≈ 0.230

	// Y方向に収めるために必要なZ
	float zForY = (spanY * 0.5f) / tanHalfFov;

	// X方向に収めるために必要なZ
	// 一般的なアスペクト比 16:9 ≈ 1.778 を仮定して計算
	// （実際のアスペクト比と違っても余白係数MARGINで吸収できる）
	const float ASSUMED_ASPECT = 16.0f / 9.0f;
	float zForX = (spanX * 0.5f) / (tanHalfFov * ASSUMED_ASPECT);

	float zDistance = std::max({ zForY, zForX, Z_MIN }) * MARGIN;

	cameraTransform_.translate = { centerX, centerY, -zDistance };
	camera_->SetTransform(cameraTransform_);
}

// ============================================================
//  ReloadStage
// ============================================================

void SampleScene::ReloadStage(int stageNumber) {
	stageNumber_ = stageNumber;
	No::Registry& registry = *GetRegistry();

	std::vector<No::Entity> all;
	for (auto e : registry.View<GridCellComponent>())     all.push_back(e);
	for (auto e : registry.View<PlayerComponent>())       all.push_back(e);
	for (auto e : registry.View<EnemyComponent>())        all.push_back(e);
	for (auto e : registry.View<PlayerBulletComponent>()) all.push_back(e);
	for (auto e : registry.View<AmmoItemComponent>())     all.push_back(e);

	for (auto e : all) {
		registry.DestroyEntity(e);
	}
	
	// 削除を即座に実行
	registry.FlushDestroy();

	// ---- 指定ステージを読み込んで再初期化 ----
	std::string path = "resources/game/td_3105/Stages/stage_0"
		+ std::to_string(stageNumber_) + ".json";
	MapData::StageData stageData = MapLoader::LoadStage(path);

	// ロードしたスケールをGridUtilsに設定
	GridUtils::gGridScale = stageData.connectionMap.gridScale;

	InitializeGrid(registry, stageData.connectionMap);
	for (const auto& entity : stageData.entityMap.entities) {
		if (entity.type == "player")     InitializePlayer(registry, entity.x, entity.y);
		else if (entity.type == "enemy") InitializeEnemy(registry, entity.x, entity.y);
	}

	// ステージが変わるたびにカメラを再配置
	SetupCameraForStage(stageData.connectionMap);
}

// ============================================================
//  OnEnterEditorMode
// ============================================================

void SampleScene::OnEnterEditorMode() {
#ifdef USE_IMGUI
	// エディタを空のキャンバスにリセット
	editor_->Reset();
#endif
}

// ============================================================
//  OnExitEditorMode
// ============================================================

void SampleScene::OnExitEditorMode() {
#ifdef USE_IMGUI
	// 現在のステージ番号でゲームに戻る
	ReloadStage(stageNumber_);
#endif
}

// ============================================================
//  UpdateGame
// ============================================================

void SampleScene::UpdateGame(No::Registry& registry) {
	///それぞれの判定はコメントアウトするが残しておく
	// ToDo : クリア・ゲームオーバーの判定はいつでも使えるようにするが、今は必要ない。
	(void)registry;		//今は未使用なので警告回避

	//// ---- ゲームオーバー判定 ----
	//auto playerView = registry.View<PlayerTag, DeathFlag>();
	//for (auto entity : playerView) {
	//	auto* flag = registry.GetComponent<DeathFlag>(entity);
	//	if (flag && flag->isDead) {
	//		ReloadStage(stageNumber_);
	//		return;
	//	}
	//}

	// ---- クリア判定（敵が全滅したとき） ----
	//auto enemyView = registry.View<EnemyTag, DeathFlag>();
	//if (enemyView.Empty()) return;

	//int aliveEnemies = 0;
	//for (auto entity : enemyView) {
	//	auto* flag = registry.GetComponent<DeathFlag>(entity);
	//	if (flag && !flag->isDead) aliveEnemies++;
	//}

	//if (aliveEnemies == 0) {
	//	ReloadStage(stageNumber_ + 1);
	//}
}

// ============================================================
//  DebugStageControlUI
// ============================================================

void SampleScene::DebugStageControlUI(No::Registry& registry) {
#ifdef USE_IMGUI
	(void)registry;
	ImGui::Begin("Stage Control");

	ImGui::Text("Current Stage: %d", stageNumber_);
	ImGui::Separator();

	// ---- ステージ指定リロード ----
	static int inputStageNumber = 1;
	ImGui::InputInt("Stage Number", &inputStageNumber);
	if (inputStageNumber < 1) inputStageNumber = 1;

	if (ImGui::Button("Load Stage")) {
		if (!isEditorMode_) ReloadStage(inputStageNumber);
	}
	ImGui::SameLine();
	if (ImGui::Button("Reload Current")) {
		if (!isEditorMode_) ReloadStage(stageNumber_);
	}

	ImGui::Separator();

	// ---- エディタモード切り替え ----
	bool prevEditorMode = isEditorMode_;
	ImGui::Checkbox("Editor Mode", &isEditorMode_);
	if (isEditorMode_ != prevEditorMode) {
		if (isEditorMode_) {
			OnEnterEditorMode();
		} else {
			OnExitEditorMode();
		}
	}

	ImGui::End();
#else

	// USE_IMGUI が定義されていないとき
	(void)registry; // 警告回避

#endif
}

// ============================================================
//  NotSystemUpdate
// ============================================================

void SampleScene::NotSystemUpdate() {

#ifdef USE_IMGUI
	No::Registry& registry = *GetRegistry();

	// カメラ手動調整ウィンドウ（自動配置後の微調整用）
	ImGui::Begin("camera");
	ImGui::DragFloat3("pos", &cameraTransform_.translate.x, 0.1f);
	ImGui::DragFloat3("rot", &cameraTransform_.rotation.x, 0.1f);
	ImGui::End();
	camera_->SetTransform(cameraTransform_);

	DebugStageControlUI(registry);

	// エディタモード中はエディタの Update を呼ぶ
	if (isEditorMode_) {
		editor_->Update(registry);
	}

	// エディタモード中はゲーム判定チェックをスキップ
	if (!isEditorMode_) {
		UpdateGame(registry);
	}
#endif

	camera_->Update();

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
	// kEnemy を追加：敵同士の衝突も検出する
	collider->collideMask = kPlayer | kPlayerBullet | kEnemy;

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