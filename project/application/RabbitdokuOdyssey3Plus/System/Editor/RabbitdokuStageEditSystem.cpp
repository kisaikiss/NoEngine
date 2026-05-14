#include "stdafx.h"
#include "RabbitdokuStageEditSystem.h"
#include "application/RabbitdokuOdyssey3Plus/Game/RabbitdokuCollisionLayer.h"
namespace {
int count = 0;
}

void RabbitdokuStageEditSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
#ifdef USE_IMGUI
	if (No::Mouse::IsPress(No::MouseButton::Left)) {
		if (!No::IsMouseOverSceneWindow())return;
		AddBlock(registry);
	
	}
#else
	static_cast<void>(registry);
#endif // USE_IMGUI


}

void RabbitdokuStageEditSystem::AddBlock(No::Registry& registry) {
	No::Entity e = registry.GenerateEntity();
	auto* transform = registry.AddComponent<No::Transform2DComponent>(e);
	transform->translate = No::Get2DSceneMousePosition(registry);
	auto* sprite = registry.AddComponent<No::SpriteComponent>(e);
	auto* collider = registry.AddComponent<No::AABBCollider2D>(e);
	
	registry.AddComponent<No::CollisionBody>(e)->type = No::BodyType::Static;
	registry.AddComponent<RabbitdokuCollisionLayerComponent>(e)->layer = RabbitdokuCollisionLayerComponent::Terrain;

	sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/RabbitdokuOdyssey3Plus/Sprite/block.png");
	transform->scale.x = static_cast<float>(sprite->textureHandle.GetWidth());
	transform->scale.y = static_cast<float>(sprite->textureHandle.GetHeight());
	collider->max = transform->scale / 2.f;
	collider->min = -transform->scale / 2.f;
}
