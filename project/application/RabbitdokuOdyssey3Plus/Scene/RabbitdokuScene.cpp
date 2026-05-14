#include "stdafx.h"
#include "RabbitdokuScene.h"

#include "../System/Player/RabbitdokuMoveSystem.h"
#include "../System/RabbitdokuCollisionEventSystem.h"
#include "../System/Player/RabbitdokuPushBackSystem.h"
#include "../System/FollowCamera2DSystem.h"
#include "../System/Camera2DChangeSystem.h"
#include "../System/Editor/RabbitdokuStageEditSystem.h"

#include "../Component/RabbitdokuComponent.h"
#include "../Component/FollowCamera2DComponent.h"
#include "../Component/RoomComponent.h"
#include "../Game/RabbitdokuCollisionLayer.h"


void RabbitdokuScene::Setup() {
	auto& registry = *GetRegistry();
	AddSystems();
	InitPlayer(registry);
	InitCamera(registry);
	InitBlock(registry);
	InitRoom(registry);
}

void RabbitdokuScene::AddSystems() {
	AddSystem(std::make_unique<No::EditSystem>());
	AddSystem(std::make_unique<RabbitdokuStageEditSystem>());
	AddSystem(std::make_unique<No::DebugCamera2DSystem>());
	AddSystem(std::make_unique<Camera2DChangeSystem>());

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
	
	AddSystem(std::make_unique<No::SpriteAnimationSystem>());
}

void RabbitdokuScene::InitPlayer(No::Registry& registry) {
	No::Entity e = registry.GenerateEntity();
	auto* transform =  registry.AddComponent<No::Transform2DComponent>(e);
	registry.AddComponent<No::Velocity2DComponent>(e);
	registry.AddComponent<Rabbitdoku>(e);
	registry.AddComponent<No::EditTag>(e)->name = "Rabbitdoku";
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
	animator->animeFrameHeight = 64.f;
	animator->animeFrameWidth = 64.f;
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

void RabbitdokuScene::InitBlock(No::Registry& registry) {
	for (uint32_t i = 0; i < 21; i++) {
		if (i % 2 == 0) continue;
		No::Entity e = registry.GenerateEntity();
		auto* transform = registry.AddComponent<No::Transform2DComponent>(e);
		transform->translate.x = float(i) * 64.f;
		transform->translate.y = 640.f;
		auto* sprite = registry.AddComponent<No::SpriteComponent>(e);
		auto* collider = registry.AddComponent<No::AABBCollider2D>(e);
		collider->max = 32.f;
		collider->min = -32.f;
		registry.AddComponent<No::CollisionBody>(e)->type = No::BodyType::Static;
		registry.AddComponent<RabbitdokuCollisionLayerComponent>(e)->layer = RabbitdokuCollisionLayerComponent::Terrain;

		sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/RabbitdokuOdyssey3Plus/Sprite/block.png");
		transform->scale.x = static_cast<float>(sprite->textureHandle.GetWidth());
		transform->scale.y = static_cast<float>(sprite->textureHandle.GetHeight());
	}

	for (uint32_t i = 0; i < 41; i++) {

		No::Entity e = registry.GenerateEntity();
		auto* transform = registry.AddComponent<No::Transform2DComponent>(e);
		transform->translate.x = float(i) * 64.f;
		transform->translate.y = 704.f;
		auto* sprite = registry.AddComponent<No::SpriteComponent>(e);
		auto* collider = registry.AddComponent<No::AABBCollider2D>(e);
		collider->max = 32.f;
		collider->min = -32.f;
		registry.AddComponent<No::CollisionBody>(e)->type = No::BodyType::Static;
		registry.AddComponent<RabbitdokuCollisionLayerComponent>(e)->layer = RabbitdokuCollisionLayerComponent::Terrain;

		sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/RabbitdokuOdyssey3Plus/Sprite/block.png");
		transform->scale.x = static_cast<float>(sprite->textureHandle.GetWidth());
		transform->scale.y = static_cast<float>(sprite->textureHandle.GetHeight());
	}

	for (uint32_t i = 0; i < 20; i++) {

		No::Entity e = registry.GenerateEntity();
		auto* transform = registry.AddComponent<No::Transform2DComponent>(e);
		transform->translate.x = float(i) * 64.f;
		transform->translate.y = 0.f;
		auto* sprite = registry.AddComponent<No::SpriteComponent>(e);
		auto* collider = registry.AddComponent<No::AABBCollider2D>(e);
		collider->max = 32.f;
		collider->min = -32.f;
		registry.AddComponent<No::CollisionBody>(e)->type = No::BodyType::Static;
		registry.AddComponent<RabbitdokuCollisionLayerComponent>(e)->layer = RabbitdokuCollisionLayerComponent::Terrain;

		sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/RabbitdokuOdyssey3Plus/Sprite/block.png");
		transform->scale.x = static_cast<float>(sprite->textureHandle.GetWidth());
		transform->scale.y = static_cast<float>(sprite->textureHandle.GetHeight());
	}

	for (uint32_t i = 0; i < 11; i++) {

		No::Entity e = registry.GenerateEntity();
		auto* transform = registry.AddComponent<No::Transform2DComponent>(e);
		transform->translate.x = 0.f;
		transform->translate.y = float(i) * 64.f;
		auto* sprite = registry.AddComponent<No::SpriteComponent>(e);
		auto* collider = registry.AddComponent<No::AABBCollider2D>(e);
		collider->max = 32.f;
		collider->min = -32.f;
		registry.AddComponent<No::CollisionBody>(e)->type = No::BodyType::Static;
		registry.AddComponent<RabbitdokuCollisionLayerComponent>(e)->layer = RabbitdokuCollisionLayerComponent::Terrain;

		sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/RabbitdokuOdyssey3Plus/Sprite/block.png");
		transform->scale.x = static_cast<float>(sprite->textureHandle.GetWidth());
		transform->scale.y = static_cast<float>(sprite->textureHandle.GetHeight());
	}

	for (uint32_t i = 0; i < 11; i++) {
		if (i % 2 == 0) continue;
		No::Entity e = registry.GenerateEntity();
		auto* transform = registry.AddComponent<No::Transform2DComponent>(e);
		transform->translate.x = 64.f;
		transform->translate.y = float(i) * 64.f;
		auto* sprite = registry.AddComponent<No::SpriteComponent>(e);
		auto* collider = registry.AddComponent<No::AABBCollider2D>(e);
		collider->max = 32.f;
		collider->min = -32.f;
		registry.AddComponent<No::CollisionBody>(e)->type = No::BodyType::Static;
		registry.AddComponent<RabbitdokuCollisionLayerComponent>(e)->layer = RabbitdokuCollisionLayerComponent::Terrain;

		sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/RabbitdokuOdyssey3Plus/Sprite/block.png");
		transform->scale.x = static_cast<float>(sprite->textureHandle.GetWidth());
		transform->scale.y = static_cast<float>(sprite->textureHandle.GetHeight());
	}

	for (uint32_t i = 0; i < 6; i++) {

		No::Entity e = registry.GenerateEntity();
		auto* transform = registry.AddComponent<No::Transform2DComponent>(e);
		transform->translate.x = 1280.f;
		transform->translate.y = float(i) * 64.f;
		auto* sprite = registry.AddComponent<No::SpriteComponent>(e);
		auto* collider = registry.AddComponent<No::AABBCollider2D>(e);
		collider->max = 32.f;
		collider->min = -32.f;
		registry.AddComponent<No::CollisionBody>(e)->type = No::BodyType::Static;
		registry.AddComponent<RabbitdokuCollisionLayerComponent>(e)->layer = RabbitdokuCollisionLayerComponent::Terrain;

		sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/RabbitdokuOdyssey3Plus/Sprite/block.png");
		transform->scale.x = static_cast<float>(sprite->textureHandle.GetWidth());
		transform->scale.y = static_cast<float>(sprite->textureHandle.GetHeight());
	}
}

void RabbitdokuScene::InitRoom(No::Registry& registry) {
	// room1
	{
		auto e = registry.GenerateEntity();
		auto* room = registry.AddComponent<RoomComponent>(e);
		room->bounds.top = 0.0f;
		room->bounds.left = 0.0f;
		room->bounds.right = 1280.f;
		room->bounds.bottom = 720.f;
	}
	
	// room2
	{
		auto e = registry.GenerateEntity();
		auto* room = registry.AddComponent<RoomComponent>(e);
		room->bounds.top = 0.0f;
		room->bounds.left = 1280.f;
		room->bounds.right = 3280.f;
		room->bounds.bottom = 720.f;
	}

}

void RabbitdokuScene::NotSystemUpdate() {

}
