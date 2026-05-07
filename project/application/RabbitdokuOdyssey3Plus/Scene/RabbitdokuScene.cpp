#include "stdafx.h"
#include "RabbitdokuScene.h"
#include "../System/RabbitdokuMoveSystem.h"
#include "../Component/RabbitdokuComponent.h"

void RabbitdokuScene::Setup() {
	auto& registry = *GetRegistry();
	AddSystems();
	InitPlayer(registry);
}

void RabbitdokuScene::AddSystems() {
	AddSystem(std::make_unique<No::EditSystem>());

	AddSystem(std::make_unique<RabbitdokuMoveSystem>());

	AddSystem(std::make_unique<No::MovementSystem>());
	AddSystem(std::make_unique<No::SpriteAnimationSystem>());
}

void RabbitdokuScene::InitPlayer(No::Registry& registry) {
	No::Entity e = registry.GenerateEntity();
	auto* transform =  registry.AddComponent<No::Transform2DComponent>(e);
	registry.AddComponent<No::Velocity2DComponent>(e);
	registry.AddComponent<Rabbitdoku>(e);
	registry.AddComponent<No::EditTag>(e)->name = "Rabbitdoku";
	auto* sprite = registry.AddComponent<No::SpriteComponent>(e);
	sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/flower.png");
	transform->scale.x = static_cast<float>(sprite->textureHandle.GetWidth());
	transform->scale.y = static_cast<float>(sprite->textureHandle.GetHeight());
}
