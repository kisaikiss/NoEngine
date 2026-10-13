#include "stdafx.h"
#include "GameScene.h"


#include "../Component/Stage/StageComponent.h"
#include "../Component/Game/GameProgressComponent.h"
#include "../System/Game/CollisionLayer.h"

void GameScene::Setup() {
	auto& registry = *GetRegistry();

	// 進行状況管理用Entity（総数/取得数/経過時間）
	{
		auto e = registry.GenerateEntity();
		registry.AddComponent<GameProgressComponent>(e);
		registry.AddComponent<No::EditTag>(e)->name = "GameProgress";
	}

	// 地形
	{
		auto e = registry.GenerateEntity();
		registry.AddComponent<No::TerrainMesh>(e);
		auto* bm = registry.AddComponent<No::MeshComponent>(e);
		registry.AddComponent<StageComponent>(e)->stageColliderName = "resources/game/ClockworksDisease/Model/StageMap/testStage.obj";
		registry.AddComponent<CollisionLayerComponent>(e)->layer = CollisionLayerComponent::Terrain;
		bm->meshName = "resources/game/ClockworksDisease/Model/StageMap/testStage.obj";
		registry.AddComponent<No::MaterialComponent>(e);
		registry.AddComponent<No::TransformComponent>(e);
		registry.AddComponent<No::EditTag>(e)->name = "stage";
	}

}