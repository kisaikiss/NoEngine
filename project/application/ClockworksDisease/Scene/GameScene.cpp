#include "stdafx.h"
#include "GameScene.h"

#include "../System/Player/PlayerMoveSystem.h"
#include "../System/Camera/FollowCameraSystem.h"
#include "../System/Game/CollisionEventSystem.h"
#include "../System/Player/PlayerPushBackSystem.h"
#include "../System/Player/PlayerLevelUpSystem.h"
#include "../System/Object/BoxColliderUpdateSystem.h"

#include "../Component/Player/PlayerComponent.h"
#include "../Component/Camera/FollowCameraComponent.h"
#include "../System/Game/CollisionLayer.h"
#include "../System/Game/ColliderDrawSystem.h"
#include "../System/Game/ItemGetSystem.h"
#include "../System/Object/BigPowerItemSystem.h"
#include "../System/UI/LevelUISystem.h"
#include "../System/UI/StaminaUISystem.h"

void GameScene::Setup() {
	AddSystems();
	auto& registry = *GetRegistry();

	// 地形
	// ToDo: コンストラクタかエディタ上で指定できるようにする
	{
		auto e = registry.GenerateEntity();
		auto* t = registry.AddComponent<No::TerrainMesh>(e);
		auto* bm = registry.AddComponent<No::MeshComponent>(e);
		No::LoadMeshCollider("resources/game/ClockworksDisease/Model/StageMap/testStage.obj", t);
		registry.AddComponent<CollisionLayerComponent>(e)->layer = CollisionLayerComponent::Terrain;
		bm->meshName = "resources/game/ClockworksDisease/Model/StageMap/testStage.obj";
		registry.AddComponent<No::MaterialComponent>(e)->color = No::Color(0.3f, 0.3f, 0.3f);
		registry.AddComponent<No::TransformComponent>(e);
	}

}

void GameScene::AddSystems() {
	AddSystem(std::make_unique<No::EditSystem>());

	AddSystem(std::make_unique<No::ModelLoadSystem>());
	AddSystem(std::make_unique<No::SpriteLoadSystem>());
	AddSystem(std::make_unique<No::AnimationSystem>());
	AddSystem(std::make_unique<BoxColliderUpdateSystem>());
	AddSystem(std::make_unique<PlayerMoveSystem>());
	AddSystem(std::make_unique<StaminaUISystem>());
	AddSystem(std::make_unique<No::GroundResetSystem>());

	AddSystem(std::make_unique<No::NarrowPhaseSystem>());
	AddSystem(std::make_unique<No::CollisionResolutionSystem>());
	AddSystem(std::make_unique<CollisionEventSystem>());
	AddSystem(std::make_unique<PlayerPushBackSystem>());
	AddSystem(std::make_unique<ItemGetSystem>());
	AddSystem(std::make_unique<BigPowerItemSystem>());
	AddSystem(std::make_unique<PlayerLevelUpSystem>());
	AddSystem(std::make_unique<LevelUISystem>());
	AddSystem(std::make_unique<No::EffectEmitSystem>());
	AddSystem(std::make_unique<No::ParticleEmitterSystem>());
	AddSystem(std::make_unique<No::ParticleSystem>());
	AddSystem(std::make_unique<No::MovementSystem>());
	AddSystem(std::make_unique<ColliderDrawSystem>());
	AddSystem(std::make_unique<FollowCameraSystem>());
	AddSystem(std::make_unique<No::SpriteAnimationSystem>());
	AddSystem(std::make_unique<No::DrawManipulatorSystem>());
	AddSystem(std::make_unique<No::DebugCameraSystem>());
	AddSystem(std::make_unique<No::CameraSystem>());
}
