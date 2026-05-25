#include "stdafx.h"
#include "RabbitdokuScene.h"

#include "../System/Player/RabbitdokuMoveSystem.h"
#include "../System/RabbitdokuCollisionEventSystem.h"
#include "../System/Player/RabbitdokuPushBackSystem.h"
#include "../System/FollowCamera2DSystem.h"
#include "../System/Camera2DChangeSystem.h"
#include "../System/Editor/RabbitdokuStageEditSystem.h"
#include "../System/Game/RabbitdokuLoadSystem.h"
#include "../System/Game/RabbitdokuSceneResetSystem.h"
#include "../System/Game/RabbitdokuItemGetSystem.h"

#include "../Component/RabbitdokuComponent.h"
#include "../Component/FollowCamera2DComponent.h"
#include "../Component/SaveDataComponent.h"
#include "../Game/RabbitdokuCollisionLayer.h"
#include "../Game/RabbitdokuTag.h"


void RabbitdokuScene::Setup() {
	auto& registry = *GetRegistry();
	AddSystems();
	InitPlayer(registry);
	InitCamera(registry);
}

void RabbitdokuScene::AddSystems() {
	AddSystem(std::make_unique<No::EditSystem>());
	AddSystem(std::make_unique<No::SpriteLoadSystem>());
	AddSystem(std::make_unique<RabbitdokuStageEditSystem>());
	AddSystem(std::make_unique<No::DebugCamera2DSystem>());
	AddSystem(std::make_unique<Camera2DChangeSystem>());

	AddSystem(std::make_unique<RabbitdokuLoadSystem>());

	AddSystem(std::make_unique<RabbitdokuMoveSystem>());
	AddSystem(std::make_unique<FollowCamera2DSystem>());

	AddSystem(std::make_unique<No::GroundResetSystem>());

	AddSystem(std::make_unique<No::Movement2DSystem>(No::Movement2DSystem::MovementAxis::Horizontal));
	AddSystem(std::make_unique<No::NarrowPhase2DSystem>(No::NarrowPhase2DSystem::TestAxis::Horizontal));
	AddSystem(std::make_unique<No::CollisionResolution2DSystem>());

	AddSystem(std::make_unique<No::Movement2DSystem>(No::Movement2DSystem::MovementAxis::Vertical));
	AddSystem(std::make_unique<No::NarrowPhase2DSystem>(No::NarrowPhase2DSystem::TestAxis::Vertical));
	AddSystem(std::make_unique<No::CollisionResolution2DSystem>());

	AddSystem(std::make_unique<RabbitdokuCollisionEventSystem>());
	AddSystem(std::make_unique<RabbitdokuPushBackSystem>());
	AddSystem(std::make_unique<RabbitItemGetSystem>());
	
	AddSystem(std::make_unique<No::SpriteAnimationSystem>());
	AddSystem(std::make_unique<No::Camera2DSystem>());


	AddSystem(std::make_unique<RabbitdokuSceneResetSystem>());
}

void RabbitdokuScene::InitPlayer(No::Registry& registry) {
	No::Entity e = registry.GenerateEntity();
	auto* transform =  registry.AddComponent<No::Transform2DComponent>(e);
	registry.AddComponent<No::Velocity2DComponent>(e);
	registry.AddComponent<Rabbitdoku>(e);
	registry.AddComponent<No::EditTag>(e)->name = "Rabbitdoku";
	registry.AddComponent<SaveDataComponent>(e);
	auto* collider = registry.AddComponent<No::AABBCollider2D>(e);
	collider->max.y = 32.f;
	collider->min.y = -24.f;
	collider->max.x = 24.f;
	collider->min.x = -24.f;
	registry.AddComponent<No::GroundStateComponent>(e);
	registry.AddComponent<No::CollisionBody>(e)->type = No::BodyType::Dynamic;
	registry.AddComponent<RabbitdokuCollisionLayerComponent>(e)->layer = RabbitdokuCollisionLayerComponent::Player;
	auto* sprite = registry.AddComponent<No::SpriteComponent>(e);
	sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/RabbitdokuOdyssey3Plus/Sprite/Player.png");
	transform->scale.x = 64.f;
	transform->scale.y = 64.f;
	auto* animator = registry.AddComponent<No::Animator2DComponent>(e);
	animator->animeFrameHeight = 32.f;
	animator->animeFrameWidth = 32.f;
	animator->framesNum = 2;
	animator->frameByFrameTime = 0.3f;
}

void RabbitdokuScene::InitCamera(No::Registry& registry) {
	{
		No::Entity e = registry.GenerateEntity();
		registry.AddComponent<No::Transform2DComponent>(e);
		registry.AddComponent<No::EditTag>(e)->name = "Camera";
		registry.AddComponent<No::Camera2DComponent>(e);
		registry.AddComponent<No::ActiveCamera2DTag>(e);
		registry.AddComponent<FollowCamera2DComponent>(e);
	}
	
	{
		No::Entity e = registry.GenerateEntity();
		registry.AddComponent<No::Transform2DComponent>(e);
		registry.AddComponent<No::EditTag>(e)->name = "DebugCamera";
		registry.AddComponent<No::Camera2DComponent>(e);
		registry.AddComponent<No::DebugCamera2DComponent>(e);
	}
	
}


void RabbitdokuScene::InitRoom(No::Registry& registry) {
	// room1
	{
		auto e = registry.GenerateEntity();
		registry.AddComponent<RoomTag>(e);
		registry.AddComponent<No::EditTag>(e)->name = "room";
		auto* collider = registry.AddComponent<No::AABBCollider2D>(e);
		collider->max = No::Vector2(640.f, 360.f);
		collider->min = -No::Vector2(640.f, 360.f);
		registry.AddComponent<No::Transform2DComponent>(e)->translate = No::Vector2(640.f, 360.f);
	}
	
	// room2
	{
		auto e = registry.GenerateEntity();
		registry.AddComponent<RoomTag>(e);
		registry.AddComponent<No::EditTag>(e)->name = "room";
		auto* collider = registry.AddComponent<No::AABBCollider2D>(e);
		collider->max = No::Vector2(640.f, 360.f);
		collider->min = -No::Vector2(640.f, 360.f);
		registry.AddComponent<No::Transform2DComponent>(e)->translate = No::Vector2(640.f + 1280.f, 360.f);
	}

}

void RabbitdokuScene::NotSystemUpdate() {
}
