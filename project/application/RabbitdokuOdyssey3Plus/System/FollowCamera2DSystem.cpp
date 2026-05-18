#include "stdafx.h"
#include "FollowCamera2DSystem.h"
#include "../Component/FollowCamera2DComponent.h"
#include "../Component/RabbitdokuComponent.h"
#include "../Game/RabbitdokuTag.h"

void FollowCamera2DSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto cameraView = registry.View<No::ActiveCamera2DTag, FollowCamera2DComponent, No::Camera2DComponent>();
	auto playerView = registry.View<Rabbitdoku, No::Transform2DComponent>();
	No::Vector2 playerPosition{};
	for (auto e : playerView) {
		playerPosition = registry.GetComponent<No::Transform2DComponent>(e)->translate;
	}

	No::Camera2DComponent* camera = nullptr;
	No::Transform2DComponent* transform = nullptr;
	FollowCamera2DComponent* followCamera = nullptr;
	for (auto e : cameraView) {
		transform = registry.GetComponent<No::Transform2DComponent>(e);
		followCamera = registry.GetComponent<FollowCamera2DComponent>(e);
		followCamera->lerpPosition = No::Lerp(transform->translate, playerPosition, 0.5f);
		camera = registry.GetComponent<No::Camera2DComponent>(e);
	}

	
	if (camera == nullptr || transform == nullptr || followCamera == nullptr) return;
	No::Entity roomE = FindRoom(registry, playerPosition);

	if (roomE == No::INVALID_ENTITY) return;
	
	auto room = GetWorldAABB(registry.GetComponent<No::AABBCollider2D>(roomE), registry.GetComponent<No::Transform2DComponent>(roomE));
	if (followCamera->lerpPosition.x + camera->width / 2.f > room.max.x) {
		followCamera->lerpPosition.x = room.max.x - camera->width / 2.f;
	}
	if (followCamera->lerpPosition.x - camera->width / 2.f < room.min.x) {
		followCamera->lerpPosition.x = room.min.x + camera->width / 2.f;
	}
	if (followCamera->lerpPosition.y + camera->height / 2.f > room.max.y) {
		followCamera->lerpPosition.y = room.max.y - camera->height / 2.f;
	}
	if (followCamera->lerpPosition.y - camera->height / 2.f < room.min.y) {
		followCamera->lerpPosition.y = room.min.y + camera->height / 2.f;
	}
	transform->translate = No::Lerp(transform->translate, followCamera->lerpPosition, 0.1f);

}

No::Entity FollowCamera2DSystem::FindRoom(No::Registry& registry, const No::Vector2& pos) {
	auto view = registry.View<RoomTag, No::Transform2DComponent, No::AABBCollider2D>();
	for (auto e : view) {
		if (No::IsCollision(pos, registry.GetComponent<No::AABBCollider2D>(e), registry.GetComponent<No::Transform2DComponent>(e))) return e;
	}
	return No::INVALID_ENTITY;
}

No::AABBCollider2D FollowCamera2DSystem::GetWorldAABB(No::AABBCollider2D* box, No::Transform2DComponent::Transform2D* transform) {
	No::AABBCollider2D result{};
	result.max = box->max + transform->translate;
	result.min = box->min + transform->translate;
	return result;
}
