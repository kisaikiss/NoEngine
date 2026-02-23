#include "SampleScene.h"
#include "../Component/GridCellComponent.h"
#include "../Component/PlayerComponent.h"
#include "../Component/EnemyComponent.h"
#include "../Component/HealthComponent.h"
#include "../GameTag.h"
#include "../System/GridRenderSystem.h"
#include "../System/PlayerMovementSystem.h"
#include "../System/PlayerWeaponSystem.h"
#include "../System/PlayerBulletSystem.h"
#include "../System/AmmoItemSystem.h"
#include "../System/EnemyMovementSystem.h"
#include "../System/EnemyCollisionSystem.h"
#include "../MapData/ShinMapData.h"
#include <vector>

void SampleScene::Setup() {
	// システム追加順序は重要。
	// PlayerMovement が isMoving を更新した後に EnemyMovement が読む必要がある。
	// DestroyGameObject（相当）は全衝突判定の後。
	AddSystem(std::make_unique<GridRenderSystem>());
	AddSystem(std::make_unique<PlayerMovementSystem>());
	AddSystem(std::make_unique<PlayerWeaponSystem>());
	AddSystem(std::make_unique<PlayerBulletSystem>());
	AddSystem(std::make_unique<AmmoItemSystem>());
	AddSystem(std::make_unique<EnemyMovementSystem>());    // Player の isMoving 確定後
	AddSystem(std::make_unique<EnemyCollisionSystem>());   // 移動後に衝突判定

	// レジストリ取得
	No::Registry& registry = *GetRegistry();

	// エンティティ初期化
	InitializeGrid(registry);

	// プレイヤーの初期位置をここ1か所で設定する。
	// グリッド座標だけ指定すれば、ワールド座標は PlayerMovementSystem が自動反映する。
	InitializePlayer(registry, 2, 2);

	// 敵を2体生成（座標はここで一元管理）
	InitializeEnemy(registry, 0, 0);
	InitializeEnemy(registry, 4, 4);

	InitializeLight(registry);


	// カメラ初期化
	camera_ = std::make_unique<NoEngine::Camera>();
	cameraTransform_.translate = { 1.0f, 1.0f, -15.0f }; // 申の字マップ中央を見る適当な位置
	camera_->SetTransform(cameraTransform_);
	SetCamera(camera_.get());
}

void SampleScene::InitializeGrid(No::Registry& registry) {
	for (size_t i = 0; i < MapData::SHIN_MAP_SIZE; ++i) {
		auto entity = registry.GenerateEntity();
		auto* cell = registry.AddComponent<GridCellComponent>(entity);

		cell->gridX = MapData::SHIN_MAP[i].x;
		cell->gridY = MapData::SHIN_MAP[i].y;
		cell->hasConnectionUp = MapData::SHIN_MAP[i].up;
		cell->hasConnectionRight = MapData::SHIN_MAP[i].right;
		cell->hasConnectionDown = MapData::SHIN_MAP[i].down;
		cell->hasConnectionLeft = MapData::SHIN_MAP[i].left;
	}
}

void SampleScene::InitializePlayer(No::Registry& registry, int startX, int startY) {
	auto entity = registry.GenerateEntity();
	auto* player = registry.AddComponent<PlayerComponent>(entity);

	// グリッド座標だけ設定する。
	// ワールド座標（transform.translate）は PlayerMovementSystem::UpdateTransform が
	// 毎フレーム先頭で自動的に同期するため、ここで手動設定しない
	player->currentNodeX = startX;
	player->currentNodeY = startY;
	player->targetNodeX = startX;
	player->targetNodeY = startY;

	registry.AddComponent<PlayerTag>(entity);
	registry.AddComponent<DeathFlag>(entity); // 自機も死亡フラグが必要

	// HP 設定（最大HP=5）
	auto* health = registry.AddComponent<HealthComponent>(entity);
	health->currentHp = 5;
	health->maxHp = 5;

	auto* transform = registry.AddComponent<No::TransformComponent>(entity);
	// translate は設定しない（PlayerMovementSystem::UpdateTransform が第1フレームから同期）
	transform->scale = { 0.1f, 0.1f, 0.1f };

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




void SampleScene::InitializeEnemy(No::Registry& registry, int startX, int startY) {
	auto entity = registry.GenerateEntity();
	auto* enemy = registry.AddComponent<EnemyComponent>(entity);
	enemy->currentNodeX = startX;
	enemy->currentNodeY = startY;
	enemy->targetNodeX = startX;
	enemy->targetNodeY = startY;

	registry.AddComponent<EnemyTag>(entity);
	registry.AddComponent<DeathFlag>(entity);

	// HP 設定（一撃死 = デフォルトの maxHp=1 のまま）
	registry.AddComponent<HealthComponent>(entity);

	auto* transform = registry.AddComponent<No::TransformComponent>(entity);
	// translate は設定しない（EnemyMovementSystem::UpdateTransform が同期）
	transform->scale = { 0.1f, 0.1f, 0.1f };

	auto* mesh = registry.AddComponent<No::MeshComponent>(entity);
	auto* material = registry.AddComponent<No::MaterialComponent>(entity);
	// モデルはプレイヤーと同じ obj を流用（将来 Enemy 専用モデルに差し替え可能）
	NoEngine::Asset::ModelLoader::LoadModel("Enemy", "resources/game/td_3105/Model/TestPlayer/TestPlayer.obj", mesh);
	material->materials = NoEngine::Asset::ModelLoader::GetMaterial("Enemy");
	material->color = { 1.0f, 0.2f, 0.2f, 1.0f }; // 赤色で敵と自機を区別
	material->psoName = L"Renderer : Default PSO";
	material->psoId = NoEngine::Render::GetPSOID(material->psoName);
	material->rootSigId = NoEngine::Render::GetRootSignatureID(material->psoName);
}

void SampleScene::InitializeLight(No::Registry& registry) {
	auto lightEntity = registry.GenerateEntity();
	auto* light = registry.AddComponent<NoEngine::Component::DirectionalLightComponent>(lightEntity);
	light->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	light->direction = { 0.0f, -1.0f, 0.0f };
	light->intensity = 1.0f;
}

void SampleScene::NotSystemUpdate() {
#ifdef USE_IMGUI
	ImGui::Begin("camera");
	ImGui::DragFloat3("pos", &cameraTransform_.translate.x, 0.1f);
	ImGui::DragFloat3("rot", &cameraTransform_.rotation.x, 0.1f);
	ImGui::End();
	camera_->SetTransform(cameraTransform_);
#endif
	camera_->Update();
	DestroyGameObject();
}

void SampleScene::DestroyGameObject() {
	No::Registry& registry = *GetRegistry();
	if (registry.View<DeathFlag>().Empty()) return;

	// イテレータ中に DestroyEntity を呼ぶと view が無効化される可能性がある。
	// 削除予定エンティティを先にリストアップしてからまとめて削除する（安全）。
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
