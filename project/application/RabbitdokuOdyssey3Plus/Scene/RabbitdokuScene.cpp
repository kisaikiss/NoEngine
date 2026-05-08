#include "stdafx.h"
#include "RabbitdokuScene.h"
#include "../System/RabbitdokuMoveSystem.h"
#include "../System/RabbitdokuCollisionEventSystem.h"
#include "../System/RabbitdokuPushBackSystem.h"
#include "../Component/RabbitdokuComponent.h"
#include "../Game/RabbitdokuCollisionLayer.h"

void RabbitdokuScene::Setup() {
	auto& registry = *GetRegistry();
	AddSystems();
	InitPlayer(registry);
	InitBlock(registry);
}

void RabbitdokuScene::AddSystems() {
	AddSystem(std::make_unique<No::EditSystem>());

	AddSystem(std::make_unique<RabbitdokuMoveSystem>());

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
	transform->scale.x = static_cast<float>(sprite->textureHandle.GetWidth());
	transform->scale.y = static_cast<float>(sprite->textureHandle.GetHeight());
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

	for (uint32_t i = 0; i < 21; i++) {

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

	for (uint32_t i = 0; i < 11; i++) {

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
