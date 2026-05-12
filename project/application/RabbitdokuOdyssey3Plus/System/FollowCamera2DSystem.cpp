#include "stdafx.h"
#include "FollowCamera2DSystem.h"
#include "../Component/FollowCamera2DComponent.h"
#include "../Component/RabbitdokuComponent.h"
#include "../Component/RoomComponent.h"

void FollowCamera2DSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto cameraView = registry.View<No::ActiveCamera2DTag, FollowCamera2DComponent>();
	auto playerView = registry.View<Rabbitdoku, No::Transform2DComponent>();
	No::Vector2 playerPosition{};
	for (auto e : playerView) {
		playerPosition = registry.GetComponent<No::Transform2DComponent>(e)->translate;
	}


	for (auto e : cameraView) {
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		transform->translate = No::Lerp(transform->translate, playerPosition, 0.1f);
	}

}

No::Entity FollowCamera2DSystem::FindRoom(No::Registry& registry, const No::Vector2& pos) {
	auto view = registry.View<RoomComponent>();
	for (auto e : view) {
		auto& room = registry.GetComponent<RoomComponent>(e)->bounds;
		if (room.Contains(pos)) return e;
	}
	return No::INVALID_ENTITY;
}
