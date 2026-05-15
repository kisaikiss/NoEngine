#include "stdafx.h"
#include "RabbitdokuStageEditSystem.h"
#include "application/RabbitdokuOdyssey3Plus/Game/RabbitdokuCollisionLayer.h"
#include "application/RabbitdokuOdyssey3Plus/Game/RabbitdokuTag.h"
namespace {
int count = 0;
}

void RabbitdokuStageEditSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
#ifdef USE_IMGUI
	DrawEditWindow();

	if (No::Mouse::IsPress(No::MouseButton::Left)) {
		if (No::IsMouseOverSceneWindow())
			AddBlock(registry);

	}
	if (No::Mouse::IsPress(No::MouseButton::Right)) {
		if (No::IsMouseOverSceneWindow())
			DeleteBlock(registry);

	}
	
	
	No::DrawGrid2D(gridSize_);
#else
	static_cast<void>(registry);
#endif // USE_IMGUI


}

void RabbitdokuStageEditSystem::AddBlock(No::Registry& registry) {
	No::Vector2 position = GetGridPosition(No::Get2DSceneMousePosition(registry));
	auto blockView = registry.View<BlockTag, No::AABBCollider2D, No::Transform2DComponent>();
	for (auto e : blockView) {
		auto* aabb = registry.GetComponent<No::AABBCollider2D>(e);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		if (No::IsCollision(position, aabb, transform)) return;
	}



	No::Entity e = registry.GenerateEntity();
	auto* transform = registry.AddComponent<No::Transform2DComponent>(e);
	transform->translate = GetGridPosition(No::Get2DSceneMousePosition(registry));
	auto* sprite = registry.AddComponent<No::SpriteComponent>(e);
	auto* collider = registry.AddComponent<No::AABBCollider2D>(e);
	registry.AddComponent<BlockTag>(e);
	registry.AddComponent<No::EditTag>(e)->name = "block" + std::to_string(count);
	registry.AddComponent<No::CollisionBody>(e)->type = No::BodyType::Static;
	registry.AddComponent<RabbitdokuCollisionLayerComponent>(e)->layer = RabbitdokuCollisionLayerComponent::Terrain;

	sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/RabbitdokuOdyssey3Plus/Sprite/block.png");
	transform->scale.x = gridSize_.x;
	transform->scale.y = gridSize_.y;
	collider->max = transform->scale / 2.f;
	collider->min = -transform->scale / 2.f;
	count++;
}

void RabbitdokuStageEditSystem::DeleteBlock(No::Registry& registry) {
	No::Vector2 position = No::Get2DSceneMousePosition(registry);
	auto blockView = registry.View<BlockTag, No::AABBCollider2D, No::Transform2DComponent>();
	for (auto e : blockView) {
		auto* aabb = registry.GetComponent<No::AABBCollider2D>(e);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		if (No::IsCollision(position, aabb, transform)) {
			registry.DestroyEntity(e);
			count--;
		}
	}
}

No::Vector2 RabbitdokuStageEditSystem::GetGridPosition(const No::Vector2& position) {
	int cellX = (int)floor(position.x / gridSize_.x);
	int cellY = (int)floor(position.y / gridSize_.y);
	float centerX = cellX * gridSize_.x + gridSize_.x * 0.5f;
	float centerY = cellY * gridSize_.y + gridSize_.y * 0.5f;

	return { centerX, centerY };
}

void RabbitdokuStageEditSystem::DrawEditWindow() {
#ifdef USE_IMGUI
	ImGui::Begin("StageEditor");
	ImGui::DragFloat2("GridSize", &gridSize_.x);
	ImGui::End();
#endif // USE_IMGUI


}
