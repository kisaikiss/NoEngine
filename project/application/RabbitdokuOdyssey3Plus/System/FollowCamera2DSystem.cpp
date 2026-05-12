#include "stdafx.h"
#include "FollowCamera2DSystem.h"
#include "../Component/FollowCamera2DComponent.h"
#include "../Component/RabbitdokuComponent.h"
#include "../Component/RoomComponent.h"

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
	auto* room = registry.GetComponent<RoomComponent>(roomE);

	if (followCamera->lerpPosition.x + camera->width / 2.f > room->bounds.right) {
		followCamera->lerpPosition.x = room->bounds.right - camera->width / 2.f;
	}
	if (followCamera->lerpPosition.x - camera->width / 2.f < room->bounds.left) {
		followCamera->lerpPosition.x = room->bounds.left + camera->width / 2.f;
	}
	if (followCamera->lerpPosition.y + camera->height / 2.f > room->bounds.bottom) {
		followCamera->lerpPosition.y = room->bounds.bottom - camera->height / 2.f;
	}
	if (followCamera->lerpPosition.y - camera->height / 2.f < room->bounds.top) {
		followCamera->lerpPosition.y = room->bounds.top + camera->height / 2.f;
	}
	transform->translate = No::Lerp(transform->translate, followCamera->lerpPosition, 0.1f);

}

No::Entity FollowCamera2DSystem::FindRoom(No::Registry& registry, const No::Vector2& pos) {
	auto view = registry.View<RoomComponent>();
	for (auto e : view) {
		auto& room = registry.GetComponent<RoomComponent>(e)->bounds;
		if (room.Contains(pos)) return e;
	}
	return No::INVALID_ENTITY;
}
