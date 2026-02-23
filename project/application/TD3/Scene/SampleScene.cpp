#include "SampleScene.h"
#include "../Component/GridCellComponent.h"
#include "../Component/PlayerComponent.h"
#include "../GameTag.h"
#include "../System/GridRenderSystem.h"
#include "../System/PlayerMovementSystem.h"
#include "../System/PlayerWeaponSystem.h"
#include "../System/PlayerBulletSystem.h"
#include "../System/AmmoItemSystem.h"
#include "../System/DeathSystem.h"
#include "../MapData/ShinMapData.h"

void SampleScene::Setup() {
	// システム追加
	AddSystem(std::make_unique<GridRenderSystem>());
	AddSystem(std::make_unique<PlayerMovementSystem>());
	AddSystem(std::make_unique<PlayerWeaponSystem>());
	AddSystem(std::make_unique<PlayerBulletSystem>());
	AddSystem(std::make_unique<AmmoItemSystem>());
	AddSystem(std::make_unique<DeathSystem>());

	// レジストリ取得
	No::Registry& registry = *GetRegistry();

	// エンティティ初期化
	InitializeGrid(registry);
	InitializePlayer(registry);
	InitializeLight(registry);

	// カメラ初期化（最後）
	camera_ = std::make_unique<NoEngine::Camera>();
	cameraTransform_.translate = { 1.0f, 1.0f, -15.0f }; // 申の字マップ中央を見る位置
	camera_->SetTransform(cameraTransform_);
	SetCamera(camera_.get());
}

void SampleScene::InitializeGrid(No::Registry& registry) {
	// 申の字マップからグリッドセルを生成
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

void SampleScene::InitializePlayer(No::Registry& registry) {
	auto entity = registry.GenerateEntity();

	// PlayerComponent（デフォルト値で初期化済み、中央ノード(1,1)）
	registry.AddComponent<PlayerComponent>(entity);

	// PlayerTag
	registry.AddComponent<PlayerTag>(entity);

	// Transform（スケールを0.1に調整）
	auto* transform = registry.AddComponent<No::TransformComponent>(entity);
	transform->translate = { 1.0f, 1.0f, 0.0f };
	transform->scale = { 0.1f, 0.1f, 0.1f };

	// Mesh
	auto* mesh = registry.AddComponent<No::MeshComponent>(entity);
	auto* material = registry.AddComponent<No::MaterialComponent>(entity);

	// モデル読み込み
	NoEngine::Asset::ModelLoader::LoadModel(
		"TestPlayer",
		"resources/game/td_3105/Model/TestPlayer/TestPlayer.obj",
		mesh
	);

	// マテリアル取得
	material->materials = NoEngine::Asset::ModelLoader::GetMaterial("TestPlayer");

	// PSO設定
	material->psoName = L"Renderer : Default PSO";
	material->psoId = NoEngine::Render::GetPSOID(material->psoName);
	material->rootSigId = NoEngine::Render::GetRootSignatureID(material->psoName);
}

void SampleScene::InitializeLight(No::Registry& registry) {
	// DirectionalLight生成
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
#endif // USE_IMGUI
	camera_->Update();
}
