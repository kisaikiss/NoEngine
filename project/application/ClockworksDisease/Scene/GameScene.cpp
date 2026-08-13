#include "stdafx.h"
#include "GameScene.h"

#include "../System/Player/PlayerStaminaSystem.h"
#include "../System/Player/PlayerJumpSystem.h"
#include "../System/Player/PlayerHorizontalMoveSystem.h"
#include "../System/Player/PlayerVerticalVelocitySystem.h"

#include "../System/Camera/FollowCameraSystem.h"
#include "../System/Game/CollisionEventSystem.h"
#include "../System/Game/TerrainLoadSystem.h"
#include "../System/Game/PlatformRideSystem.h"
#include "../System/Player/PlayerPushBackSystem.h"
#include "../System/Player/PlayerLevelUpSystem.h"
#include "../System/Object/BoxColliderUpdateSystem.h"
#include "../System/Object/MagicScaffoldSystem.h"

#include "../Component/Player/PlayerComponent.h"
#include "../Component/Camera/FollowCameraComponent.h"
#include "../Component/Stage/StageComponent.h"
#include "../System/Game/CollisionLayer.h"
#include "../System/Game/ColliderDrawSystem.h"
#include "../System/Game/ItemGetSystem.h"
#include "../System/Object/BigPowerItemSystem.h"
#include "../System/UI/LevelUISystem.h"
#include "../System/UI/StaminaUISystem.h"
#include "../System/UI/LevelUpTextSystem.h"

void GameScene::Setup() {
	AddSystems();
	auto& registry = *GetRegistry();

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

void GameScene::AddSystems() {
	AddSystem(std::make_unique<No::EditSystem>());

	AddSystem(std::make_unique<No::ModelLoadSystem>());
	AddSystem(std::make_unique<TerrainLoadSystem>());
	AddSystem(std::make_unique<No::SpriteLoadSystem>());
	AddSystem(std::make_unique<No::AnimationSystem>());
	AddSystem(std::make_unique<BoxColliderUpdateSystem>());
	AddSystem(std::make_unique<PlayerStaminaSystem>());
	AddSystem(std::make_unique<PlayerJumpSystem>());
	AddSystem(std::make_unique<PlayerHorizontalMoveSystem>());
	AddSystem(std::make_unique<PlayerVerticalVelocitySystem>());
	AddSystem(std::make_unique<StaminaUISystem>());
	AddSystem(std::make_unique<No::GroundResetSystem>());
	AddSystem(std::make_unique<MagicScaffoldSystem>());

	AddSystem(std::make_unique<No::NarrowPhaseSystem>());
	AddSystem(std::make_unique<No::CollisionResolutionSystem>());
	AddSystem(std::make_unique<CollisionEventSystem>());

	AddSystem(std::make_unique<PlayerPushBackSystem>());
	AddSystem(std::make_unique<ItemGetSystem>());

	AddSystem(std::make_unique<BigPowerItemSystem>());
	AddSystem(std::make_unique<PlayerLevelUpSystem>());
	AddSystem(std::make_unique<LevelUISystem>());
	AddSystem(std::make_unique<LevelUpTextSystem>());

	AddSystem(std::make_unique<No::EffectEmitSystem>());
	AddSystem(std::make_unique<No::ParticleEmitterSystem>());
	AddSystem(std::make_unique<No::ParticleSystem>());

	AddSystem(std::make_unique<No::CapturePlatformPrevTransformSystem>());
	AddSystem(std::make_unique<No::TransformRoutineSystem>());
	AddSystem(std::make_unique<No::MovementSystem>());
	AddSystem(std::make_unique<No::ComputePlatformDeltaSystem>());
	AddSystem(std::make_unique<PlatformRideSystem>());

	AddSystem(std::make_unique<ColliderDrawSystem>());

	AddSystem(std::make_unique<FollowCameraSystem>());

	AddSystem(std::make_unique<No::SpriteAnimationSystem>());

	AddSystem(std::make_unique<No::DrawManipulatorSystem>());
	AddSystem(std::make_unique<No::DebugCameraSystem>());
	AddSystem(std::make_unique<No::CameraSystem>());
}
