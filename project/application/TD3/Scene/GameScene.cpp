#include "GameScene.h"
#include "application/TD3/Stage/GameStateComponent.h"
#include "application/TD3/Stage/MapData.h"
#include "application/TD3/Stage/MapRenderDirtyComponent.h"
#include "application/TD3/Stage/MapTileRenderSystem.h"
#include "application/TD3/Stage/MapManager.h"

#include "application/TD3/Component/Player/RabbitComponent.h"
#include "application/TD3/System/Player/RabbitSystem.h"
#include "application/TD3/Component/GravityComponent.h"

#include "application/TD3/Utility/GameResourceComponent.h"
#include "application/TD3/Utility/InputHelperSystem.h"
#include "application/TD3/Utility/GameTag.h"
#include "application/TD3/Utility/SpriteLayer.h"
#ifdef USE_IMGUI
#include "application/TD3/Stage/MapEditorSystem.h"
#endif

namespace {
	No::Entity managerE = 0;
	No::Entity cam2dE = 0;
	No::Entity playerE = 0;
	Stage::MapManager mapManager;
}

void GameScene::Setup() {
	// ---- System 登録（実行順に並べる） ----
	AddSystem(std::make_unique<InputHelperSystem>());
#ifdef USE_IMGUI
	auto* editorSys = AddSystem(std::make_unique<MapEditorSystem>());
#endif
	auto* playerSys = AddSystem(std::make_unique<RabbitSystem>());
	auto* tileSys = AddSystem(std::make_unique<Stage::MapTileRenderSystem>());
	AddSystem(std::make_unique<No::EditSystem>());

	No::Registry& registry = *GetRegistry();

	// ---- GameManager エンティティ ----
	managerE = registry.GenerateEntity();
	registry.AddComponent<GameManagerTag>(managerE);
	registry.AddComponent<GameStateComponent>(managerE);         // editorMode = true
	auto* res = registry.AddComponent<GameResourceComponent>(managerE);
	InitializeGameResources(*res);
	registry.AddComponent<Stage::MapData>(managerE);             // 空の状態で追加
	registry.AddComponent<Stage::MapRenderDirtyComponent>(managerE);

	// ---- Camera2D エンティティ ----
	cam2dE = registry.GenerateEntity();
	registry.AddComponent<No::ActiveCamera2DTag>(cam2dE);
	registry.AddComponent<No::Camera2DComponent>(cam2dE);
	auto* cam2dTransform = registry.AddComponent<No::Transform2DComponent>(cam2dE);
	cam2dTransform->translate = { 0.f, 0.f };
	cam2dTransform->scale     = { 1.f, 1.f };

	// ---- プレイヤーエンティティ ----
	playerE = registry.GenerateEntity();
	auto* pt = registry.AddComponent<No::Transform2DComponent>(playerE);
	pt->translate = { 200.f, 200.f };
	pt->scale = { 36.f, 36.f };
	auto* ps = registry.AddComponent<No::SpriteComponent>(playerE);
	ps->textureHandle = GetGameTextureOrWhite(*res, GameResourceKey::kWhiteTexture);
	ps->layer = ToLayer(SpriteLayer::Player);
	ps->color = { 0.f, 0.8f, 1.f, 1.f };
	registry.AddComponent<RabbitComponent>(playerE);
	registry.AddComponent<GravityComponent>(playerE);
	registry.AddComponent<No::EditTag>(playerE)->name = "Player";

	// ---- System 初期化 ----
	tileSys->Initialize(managerE);
	playerSys->Initialize(managerE, playerE, cam2dE, &mapManager);
#ifdef USE_IMGUI
	editorSys->Initialize(managerE, cam2dE, playerE, &mapManager);
	// MapEditorSystem::Update の初回呼び出しでデフォルトマップを生成する
#endif
	// Release ビルド: Stage01_A を直接ロードし、InitialSpawn にプレイヤーを配置
	if (mapManager.LoadMap(registry, managerE, "Stage01_A")) {
		auto* mapData = registry.GetComponent<Stage::MapData>(managerE);
		if (mapData && pt) {
			No::Vector2 spawnPos = mapManager.GetInitialSpawnPos(*mapData);
			pt->translate = { spawnPos.x, spawnPos.y };
		}
	}


}

void GameScene::NotSystemUpdate() {}
