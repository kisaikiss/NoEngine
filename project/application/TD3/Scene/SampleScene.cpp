#include "SampleScene.h"
#include "../Component/GridCellComponent.h"
#include "../Component/PlayerComponent.h"
#include "../GameTag.h"
#include "../System/GridRenderSystem.h"
#include "../System/PlayerMovementSystem.h"
#include "../System/PlayerWeaponSystem.h"
#include "../System/PlayerBulletSystem.h"
#include "../System/AmmoItemSystem.h"
#include "../MapData/ShinMapData.h"

void SampleScene::Setup() {
	// システム追加
	// ※ DeathSystem は必ず最後に追加すること。
	//   他システムが isDead を立てた同フレームにエンティティを削除するため。
	AddSystem(std::make_unique<GridRenderSystem>());
	AddSystem(std::make_unique<PlayerMovementSystem>());
	AddSystem(std::make_unique<PlayerWeaponSystem>());
	AddSystem(std::make_unique<PlayerBulletSystem>());
	AddSystem(std::make_unique<AmmoItemSystem>());

	// レジストリ取得
	No::Registry& registry = *GetRegistry();

	// エンティティ初期化
	InitializeGrid(registry);

	// プレイヤーの初期位置をここ1か所で設定する。
	// グリッド座標だけ指定すれば、ワールド座標は PlayerMovementSystem が自動反映する。
	InitializePlayer(registry, 2, 2);

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

	auto* transform = registry.AddComponent<No::TransformComponent>(entity);
	// translate は設定しない（PlayerMovementSystem が第1フレームから正しい値を書き込む）
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

void SampleScene::DestroyGameObject()
{
	No::Registry& registry = *GetRegistry();
	//フラグないときはリターン
	if (registry.View<DeathFlag>().Empty()) { return; }
	auto view = registry.View<DeathFlag>();
	for (auto entity : view)
	{
		if (registry.Has<DeathFlag>(entity))
		{
			auto* flag = registry.GetComponent<DeathFlag>(entity);
			if (flag->isDead)
			{
				registry.DestroyEntity(entity);
			}
		}
	}
}
