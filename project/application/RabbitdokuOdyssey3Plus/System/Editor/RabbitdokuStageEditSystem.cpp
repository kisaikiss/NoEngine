#include "stdafx.h"
#include "RabbitdokuStageEditSystem.h"
#include "application/RabbitdokuOdyssey3Plus/Game/RabbitdokuCollisionLayer.h"
#include "application/RabbitdokuOdyssey3Plus/Game/RabbitdokuTag.h"
#include "../../Component/RabbitdokuComponent.h"

void RabbitdokuStageEditSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
#ifdef USE_IMGUI
	DrawEditWindow(registry);

	mousePosition_ = No::Get2DGameWindowMousePosition(registry);

	switch (state_) {
	case RabbitdokuStageEditSystem::EditState::kBlock:
		if (No::Mouse::IsPress(No::MouseButton::Left)) {
			if (No::IsMouseOverGameWindow())
				AddBlock(registry);
		}
		if (No::Mouse::IsPress(No::MouseButton::Right)) {
			if (No::IsMouseOverGameWindow())
				DeleteBlock(registry);
		}
		break;
	case RabbitdokuStageEditSystem::EditState::kRoom:
		if (No::Mouse::IsTrigger(No::MouseButton::Right)) {
			if (No::IsMouseOverGameWindow()) {
				addRoomPosition_ = No::Get2DGameWindowMousePosition(registry);
				ImGui::OpenPopup("AddRoomPopup");
			}
		}
		if (ImGui::BeginPopupContextItem("AddRoomPopup")) {
			if (ImGui::MenuItem("AddRoom")) {
				AddRoom(registry);
			}

			ImGui::EndPopup();
		}

		if (No::Mouse::IsPress(No::MouseButton::Left)) {
			if (No::IsMouseOverGameWindow()) {
				No::Entity roomE = FindRoom(registry, mousePosition_);

				if (roomE == No::INVALID_ENTITY) break;
				if (No::Mouse::IsTrigger(No::MouseButton::Left)) {
					mouseOffset_ = registry.GetComponent<No::Transform2DComponent>(roomE)->translate;
					mouseOffset_ = mouseOffset_ - mousePosition_;
				}
				auto* transform = registry.GetComponent<No::Transform2DComponent>(roomE);
				transform->translate = mousePosition_ + mouseOffset_;
			}

		}

		break;
	case RabbitdokuStageEditSystem::EditState::kGimmick:
		if (No::Mouse::IsPress(No::MouseButton::Left)) {
			if (No::IsMouseOverGameWindow()) {
				switch (gimmick_) {
				case RabbitdokuStageEditSystem::GimmickSelected::kSave:
					AddSave(registry);
					break;
				case RabbitdokuStageEditSystem::GimmickSelected::kDeathBlock:
					AddDeathBlock(registry);
					break;
				case RabbitdokuStageEditSystem::GimmickSelected::kNeedle:
					AddNeedle(registry);
					break;
				case RabbitdokuStageEditSystem::GimmickSelected::kSpring:
					AddSpring(registry);
					break;
				case RabbitdokuStageEditSystem::GimmickSelected::kCollapseBlock:
					AddCollapseBlock(registry);
					break;
				case RabbitdokuStageEditSystem::GimmickSelected::kReplenisher:
					AddReplenisher(registry);
					break;
				case RabbitdokuStageEditSystem::GimmickSelected::kDoor:
					AddDoor(registry);
					break;
				default:
					break;
				}
			}
		}

		if (No::Mouse::IsPress(No::MouseButton::Right)) {
			if (No::IsMouseOverGameWindow())
				DeleteGimmick(registry);
		}
		break;
	case EditState::kBackground:
		if (No::Mouse::IsTrigger(No::MouseButton::Right)) {
			if (No::IsMouseOverGameWindow()) {
				addRoomPosition_ = No::Get2DGameWindowMousePosition(registry);
				ImGui::OpenPopup("AddBackgroundPopup");
			}
		}
		if (ImGui::BeginPopupContextItem("AddBackgroundPopup")) {
			if (ImGui::MenuItem("AddBackground")) {
				AddBackground(registry);
			}

			ImGui::EndPopup();
		}

		if (No::Mouse::IsPress(No::MouseButton::Left)) {
			if (No::IsMouseOverGameWindow()) {
				No::Entity roomE = FindBackground(registry, mousePosition_);

				if (roomE == No::INVALID_ENTITY) break;
				if (No::Mouse::IsTrigger(No::MouseButton::Left)) {
					mouseOffset_ = registry.GetComponent<No::Transform2DComponent>(roomE)->translate;
					mouseOffset_ = mouseOffset_ - mousePosition_;
				}
				auto* transform = registry.GetComponent<No::Transform2DComponent>(roomE);
				transform->translate = mousePosition_ + mouseOffset_;
			}

		}

		break;
	case EditState::kPlayer:
		if (No::IsMouseOverGameWindow())
			if (No::Mouse::IsTrigger(No::MouseButton::Left)) {
				WarpPlayer(registry);
			}
		break;
	}


	No::DrawGrid2D(gridSize_);

	DrawRooms(registry);
	DrawCollider(registry);
#else
	static_cast<void>(registry);
#endif // USE_IMGUI


}

void RabbitdokuStageEditSystem::AddBlock(No::Registry& registry) {
	No::Vector2 position = GetGridPosition(No::Get2DGameWindowMousePosition(registry));
	auto blockView = registry.View<BlockTag, No::AABBCollider2D, No::Transform2DComponent>();
	for (auto e : blockView) {
		auto* aabb = registry.GetComponent<No::AABBCollider2D>(e);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		if (No::IsCollision(position, aabb, transform)) return;
	}



	No::Entity e = registry.GenerateEntity();
	auto* transform = registry.AddComponent<No::Transform2DComponent>(e);
	transform->translate = GetGridPosition(No::Get2DGameWindowMousePosition(registry));
	auto* sprite = registry.AddComponent<No::SpriteComponent>(e);
	sprite->layer = 10;
	auto* collider = registry.AddComponent<No::AABBCollider2D>(e);
	registry.AddComponent<BlockTag>(e);
	auto* tag = registry.AddComponent<No::EditTag>(e);
	tag->name = "block";
	tag->path = "Block/block";
	registry.AddComponent<No::CollisionBody>(e)->type = No::BodyType::Static;
	registry.AddComponent<RabbitdokuCollisionLayerComponent>(e)->layer = RabbitdokuCollisionLayerComponent::Terrain;

	sprite->textureFilePath = currentBlockTexture;
	transform->scale.x = gridSize_.x;
	transform->scale.y = gridSize_.y;
	collider->max = transform->scale / 2.f;
	collider->min = -transform->scale / 2.f;
	if (sprite->textureFilePath == BlockTextures::kBlue) {
		auto* a = registry.AddComponent<No::Animator2DComponent>(e);
		a->animeFrameHeight = 64.f;
		a->animeFrameWidth = 64.f;
		a->framesNum = 4;
		a->frameByFrameTime = 0.1f;
	}
}

void RabbitdokuStageEditSystem::AddRoom(No::Registry& registry) {

	No::Entity e = registry.GenerateEntity();
	auto* box = registry.AddComponent<RoomTag>(e);
	const No::Vector2 kRoomDefaultSize = { 1280.f,720.f };
	box->max = kRoomDefaultSize / 2.f;
	box->min = -kRoomDefaultSize / 2.f;
	registry.AddComponent<No::Transform2DComponent>(e)->translate = addRoomPosition_;

	auto* tag = registry.AddComponent<No::EditTag>(e);
	tag->name = "room";
	tag->path = "Room/room";
}

void RabbitdokuStageEditSystem::AddSave(No::Registry& registry) {
	No::Vector2 position = GetGridPosition(No::Get2DGameWindowMousePosition(registry));
	auto blockView = registry.View<BlockTag, No::AABBCollider2D, No::Transform2DComponent>();
	for (auto e : blockView) {
		auto* aabb = registry.GetComponent<No::AABBCollider2D>(e);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		if (No::IsCollision(position, aabb, transform)) return;
	}

	auto gimmickView = registry.View<GimmickTag, No::AABBCollider2D, No::Transform2DComponent>();
	for (auto e : gimmickView) {
		auto* aabb = registry.GetComponent<No::AABBCollider2D>(e);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		if (No::IsCollision(position, aabb, transform)) return;
	}


	No::Entity e = registry.GenerateEntity();
	auto* transform = registry.AddComponent<No::Transform2DComponent>(e);
	transform->translate = position;
	auto* sprite = registry.AddComponent<No::SpriteComponent>(e);
	sprite->layer = 10;
	auto* collider = registry.AddComponent<No::AABBCollider2D>(e);
	registry.AddComponent<SaveTag>(e);
	registry.AddComponent<GimmickTag>(e);
	auto* tag = registry.AddComponent<No::EditTag>(e);
	tag->name = "save";
	tag->path = "Gimmick/Save/save";
	registry.AddComponent<No::CollisionBody>(e)->type = No::BodyType::Through;
	registry.AddComponent<RabbitdokuCollisionLayerComponent>(e)->layer = RabbitdokuCollisionLayerComponent::Item;
	sprite->textureFilePath = "resources/game/RabbitdokuOdyssey3Plus/Sprite/savePoint.png";
	transform->scale.x = gridSize_.x;
	transform->scale.y = gridSize_.y;
	collider->max = transform->scale / 2.f;
	collider->min = -transform->scale / 2.f;

	auto* animator = registry.AddComponent<No::Animator2DComponent>(e);
	animator->animeFrameHeight = 64.f;
	animator->animeFrameWidth = 64.f;
	animator->framesNum = 8;
	animator->frameByFrameTime = 0.1f;
	sprite->uv.x = 0.f;
	sprite->uv.width = 1.f / 8.f;
	sprite->uv.height = 1.f;

}

void RabbitdokuStageEditSystem::AddDeathBlock(No::Registry& registry) {
	No::Vector2 position = GetGridPosition(No::Get2DGameWindowMousePosition(registry));
	auto blockView = registry.View<BlockTag, No::AABBCollider2D, No::Transform2DComponent>();
	for (auto e : blockView) {
		auto* aabb = registry.GetComponent<No::AABBCollider2D>(e);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		if (No::IsCollision(position, aabb, transform)) return;
	}

	auto gimmickView = registry.View<GimmickTag, No::AABBCollider2D, No::Transform2DComponent>();
	for (auto e : gimmickView) {
		auto* aabb = registry.GetComponent<No::AABBCollider2D>(e);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		if (No::IsCollision(position, aabb, transform)) return;
	}


	No::Entity e = registry.GenerateEntity();
	auto* transform = registry.AddComponent<No::Transform2DComponent>(e);
	transform->translate = position;
	auto* sprite = registry.AddComponent<No::SpriteComponent>(e);
	sprite->layer = 10;
	auto* collider = registry.AddComponent<No::AABBCollider2D>(e);
	registry.AddComponent<EnemyTag>(e);
	registry.AddComponent<GimmickTag>(e);
	auto* tag = registry.AddComponent<No::EditTag>(e);
	tag->name = "deathBlock";
	tag->path = "Gimmick/DeathObject/deathBlock";
	registry.AddComponent<No::CollisionBody>(e)->type = No::BodyType::Through;
	registry.AddComponent<RabbitdokuCollisionLayerComponent>(e)->layer = RabbitdokuCollisionLayerComponent::Item;
	sprite->textureFilePath = "resources/game/RabbitdokuOdyssey3Plus/Sprite/deathBlock.png";
	transform->scale.x = gridSize_.x;
	transform->scale.y = gridSize_.y;
	collider->max = transform->scale / 2.f;
	collider->min = -transform->scale / 2.f;
}

void RabbitdokuStageEditSystem::AddNeedle(No::Registry& registry) {
	No::Vector2 position = GetGridPosition(No::Get2DGameWindowMousePosition(registry));
	auto blockView = registry.View<BlockTag, No::AABBCollider2D, No::Transform2DComponent>();
	for (auto e : blockView) {
		auto* aabb = registry.GetComponent<No::AABBCollider2D>(e);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		if (No::IsCollision(position, aabb, transform)) return;
	}

	auto gimmickView = registry.View<GimmickTag, No::AABBCollider2D, No::Transform2DComponent>();
	for (auto e : gimmickView) {
		No::AABBCollider2D aabb;
		aabb.max = gridSize_ / 2.f;
		aabb.min = -gridSize_ / 2.f;
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		if (No::IsCollision(position, &aabb, transform)) return;
	}


	No::Entity e = registry.GenerateEntity();
	auto* transform = registry.AddComponent<No::Transform2DComponent>(e);
	transform->translate = position;
	auto* sprite = registry.AddComponent<No::SpriteComponent>(e);
	sprite->layer = 10;
	auto* collider = registry.AddComponent<No::AABBCollider2D>(e);
	registry.AddComponent<EnemyTag>(e);
	registry.AddComponent<GimmickTag>(e);
	auto* tag = registry.AddComponent<No::EditTag>(e);
	tag->name = "needle";
	tag->path = "Gimmick/DeathObject/needle";
	registry.AddComponent<No::CollisionBody>(e)->type = No::BodyType::Through;
	registry.AddComponent<RabbitdokuCollisionLayerComponent>(e)->layer = RabbitdokuCollisionLayerComponent::Item;
	sprite->textureFilePath = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Gimmick01.png";
	transform->scale.x = gridSize_.x;
	transform->scale.y = gridSize_.y;
	collider->max.x = transform->scale.x / 2.6f;
	collider->max.y = transform->scale.y / 2.0f;
	collider->min = -transform->scale / 2.6f;
	collider->min.y = collider->max.y / 2.f;
	auto* animator = registry.AddComponent<No::Animator2DComponent>(e);
	animator->animeFrameHeight = 64.f;
	animator->animeFrameWidth = 64.f;
	animator->currentAnimation = 0;
	animator->framesNum = 5;
	animator->frameByFrameTime = 0.1f;

	sprite->uv.width = 1.f / 5.f;
	sprite->uv.height = 1.f / 4.f;
}

void RabbitdokuStageEditSystem::AddSpring(No::Registry& registry) {
	No::Vector2 position = GetGridPosition(No::Get2DGameWindowMousePosition(registry));
	auto blockView = registry.View<BlockTag, No::AABBCollider2D, No::Transform2DComponent>();
	for (auto e : blockView) {
		auto* aabb = registry.GetComponent<No::AABBCollider2D>(e);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		if (No::IsCollision(position, aabb, transform)) return;
	}

	auto gimmickView = registry.View<GimmickTag, No::AABBCollider2D, No::Transform2DComponent>();
	for (auto e : gimmickView) {
		No::AABBCollider2D aabb;
		aabb.max = gridSize_ / 2.f;
		aabb.min = -gridSize_ / 2.f;
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		if (No::IsCollision(position, &aabb, transform)) return;
	}


	No::Entity e = registry.GenerateEntity();
	auto* transform = registry.AddComponent<No::Transform2DComponent>(e);
	transform->translate = position;
	auto* sprite = registry.AddComponent<No::SpriteComponent>(e);
	sprite->layer = 10;
	auto* collider = registry.AddComponent<No::AABBCollider2D>(e);
	registry.AddComponent<SpringComponent>(e);
	registry.AddComponent<GimmickTag>(e);
	auto* tag = registry.AddComponent<No::EditTag>(e);
	tag->name = "spring";
	tag->path = "Gimmick/Spring/spring";
	registry.AddComponent<No::CollisionBody>(e)->type = No::BodyType::Through;
	registry.AddComponent<RabbitdokuCollisionLayerComponent>(e)->layer = RabbitdokuCollisionLayerComponent::Item;
	sprite->textureFilePath = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Gimmick01.png";
	transform->scale.x = gridSize_.x;
	transform->scale.y = gridSize_.y;
	collider->max.x = transform->scale.x / 2.6f;
	collider->max.y = transform->scale.y / 2.0f;
	collider->min = -transform->scale / 2.6f;
	collider->min.y = collider->max.y / 2.f;
	auto* animator = registry.AddComponent<No::Animator2DComponent>(e);
	animator->animeFrameHeight = 64.f;
	animator->animeFrameWidth = 64.f;
	animator->currentAnimation = 3;
	animator->framesNum = 1;
	animator->frameByFrameTime = 0.1f;

	sprite->uv.y = 3.f / 4.f;
	sprite->uv.width = 1.f / 5.f;
	sprite->uv.height = 1.f / 4.f;
}

void RabbitdokuStageEditSystem::AddCollapseBlock(No::Registry& registry) {
	No::Vector2 position = GetGridPosition(No::Get2DGameWindowMousePosition(registry));
	auto blockView = registry.View<BlockTag, No::AABBCollider2D, No::Transform2DComponent>();
	for (auto e : blockView) {
		auto* aabb = registry.GetComponent<No::AABBCollider2D>(e);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		if (No::IsCollision(position, aabb, transform)) return;
	}

	auto gimmickView = registry.View<GimmickTag, No::AABBCollider2D, No::Transform2DComponent>();
	for (auto e : gimmickView) {
		auto* aabb = registry.GetComponent<No::AABBCollider2D>(e);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		if (No::IsCollision(position, aabb, transform)) return;
	}

	No::Entity e = registry.GenerateEntity();
	auto* transform = registry.AddComponent<No::Transform2DComponent>(e);
	transform->translate = GetGridPosition(No::Get2DGameWindowMousePosition(registry));
	auto* sprite = registry.AddComponent<No::SpriteComponent>(e);
	sprite->layer = 10;
	auto* collider = registry.AddComponent<No::AABBCollider2D>(e);
	registry.AddComponent<BlockTag>(e);
	registry.AddComponent<GimmickTag>(e);
	auto* tag = registry.AddComponent<No::EditTag>(e);
	tag->name = "collapseBlock";
	tag->path = "Gimmick/CollapseBlock/collapseBlock";
	registry.AddComponent<No::CollisionBody>(e)->type = No::BodyType::Static;
	registry.AddComponent<RabbitdokuCollisionLayerComponent>(e)->layer = RabbitdokuCollisionLayerComponent::Item;

	sprite->textureFilePath = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Gimmick01.png";
	transform->scale.x = gridSize_.x;
	transform->scale.y = gridSize_.y;
	collider->max = transform->scale / 2.f;
	collider->min = -transform->scale / 2.f;
	auto* a = registry.AddComponent<No::Animator2DComponent>(e);
	a->animeFrameHeight = 64.f;
	a->animeFrameWidth = 64.f;
	a->currentAnimation = 1;
	a->framesNum = 5;
	a->frameByFrameTime = 0.1f;

	sprite->uv.y = 1.f / 4.f;
	sprite->uv.width = 1.f / 5.f;
	sprite->uv.height = 1.f / 4.f;
	registry.AddComponent<CollapseBlockComponent>(e);
}

void RabbitdokuStageEditSystem::AddReplenisher(No::Registry& registry) {
	No::Vector2 position = GetGridPosition(No::Get2DGameWindowMousePosition(registry));
	auto blockView = registry.View<BlockTag, No::AABBCollider2D, No::Transform2DComponent>();
	for (auto e : blockView) {
		auto* aabb = registry.GetComponent<No::AABBCollider2D>(e);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		if (No::IsCollision(position, aabb, transform)) return;
	}

	auto gimmickView = registry.View<GimmickTag, No::AABBCollider2D, No::Transform2DComponent>();
	for (auto e : gimmickView) {
		auto* aabb = registry.GetComponent<No::AABBCollider2D>(e);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		if (No::IsCollision(position, aabb, transform)) return;
	}


	No::Entity e = registry.GenerateEntity();
	auto* transform = registry.AddComponent<No::Transform2DComponent>(e);
	transform->translate = position;
	auto* sprite = registry.AddComponent<No::SpriteComponent>(e);
	sprite->layer = 10;
	auto* collider = registry.AddComponent<No::AABBCollider2D>(e);
	registry.AddComponent<ReplenisherTag>(e);
	registry.AddComponent<GimmickTag>(e);
	auto* tag = registry.AddComponent<No::EditTag>(e);
	tag->name = "replenisher";
	tag->path = "Gimmick/Replenisher/replenisher";
	registry.AddComponent<No::CollisionBody>(e)->type = No::BodyType::Through;
	registry.AddComponent<RabbitdokuCollisionLayerComponent>(e)->layer = RabbitdokuCollisionLayerComponent::Item;
	sprite->textureFilePath = "resources/game/RabbitdokuOdyssey3Plus/Sprite/JumpDowbleItem.png";
	transform->scale.x = gridSize_.x;
	transform->scale.y = gridSize_.y;
	collider->max = transform->scale / 4.f;
	collider->min = -transform->scale / 4.f;
	auto* animator = registry.AddComponent<No::Animator2DComponent>(e);
	animator->animeFrameHeight = 64.f;
	animator->animeFrameWidth = 64.f;
	animator->currentAnimation = 0;
	animator->framesNum = 5;
	animator->frameByFrameTime = 0.1f;

	sprite->uv.width = 1.f / 5.f;
}

void RabbitdokuStageEditSystem::AddDoor(No::Registry& registry) {
	No::Vector2 position = GetGridPosition(No::Get2DGameWindowMousePosition(registry));
	auto blockView = registry.View<BlockTag, No::AABBCollider2D, No::Transform2DComponent>();
	for (auto e : blockView) {
		auto* aabb = registry.GetComponent<No::AABBCollider2D>(e);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		if (No::IsCollision(position, aabb, transform)) return;
	}

	auto gimmickView = registry.View<GimmickTag, No::AABBCollider2D, No::Transform2DComponent>();
	for (auto e : gimmickView) {
		auto* aabb = registry.GetComponent<No::AABBCollider2D>(e);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		if (No::IsCollision(position, aabb, transform)) return;
	}


	No::Entity e = registry.GenerateEntity();
	auto* transform = registry.AddComponent<No::Transform2DComponent>(e);
	transform->translate = position;
	transform->translate.y -= gridSize_.y / 2.f;
	auto* sprite = registry.AddComponent<No::SpriteComponent>(e);
	sprite->layer = 10;
	auto* collider = registry.AddComponent<No::AABBCollider2D>(e);
	registry.AddComponent<DoorComponent>(e);
	registry.AddComponent<GimmickTag>(e);
	auto* tag = registry.AddComponent<No::EditTag>(e);
	tag->name = "door";
	tag->path = "Gimmick/Door/door";
	registry.AddComponent<No::CollisionBody>(e)->type = No::BodyType::Through;
	registry.AddComponent<RabbitdokuCollisionLayerComponent>(e)->layer = RabbitdokuCollisionLayerComponent::Item;
	sprite->textureFilePath = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Door.png";
	transform->scale = 128.f;
	collider->max = transform->scale / 2.f;
	collider->min = -transform->scale / 2.f;
	auto* animator = registry.AddComponent<No::Animator2DComponent>(e);
	animator->animeFrameHeight = 128.f;
	animator->animeFrameWidth = 128.f;
	animator->currentAnimation = 0;
	animator->framesNum = 8;
	animator->frameByFrameTime = 0.1f;

	sprite->uv.width = 1.f / 8.f;
}

void RabbitdokuStageEditSystem::AddBackground(No::Registry& registry) {
	No::Entity e = registry.GenerateEntity();
	auto* t = registry.AddComponent<No::Transform2DComponent>(e);
	auto* s = registry.AddComponent<No::SpriteComponent>(e);
	s->textureHandle = NoEngine::TextureManager::LoadCovertTexture(currentBackgroundTexture_);
	s->textureFilePath = currentBackgroundTexture_;
	s->layer = 5;
	t->translate = addRoomPosition_;
	t->scale.x = static_cast<float>(s->textureHandle.GetWidth());
	t->scale.y = static_cast<float>(s->textureHandle.GetHeight());
	auto* tag = registry.AddComponent<No::EditTag>(e);
	tag->name = "background";
	tag->path = "Background/background";
	auto* box = registry.AddComponent<No::AABBCollider2D>(e);
	box->max = t->scale / 2.f;
	box->min = -t->scale / 2.f;
	registry.AddComponent<No::CollisionBody>(e)->type = No::BodyType::Through;
	registry.AddComponent<BackgroundTag>(e);
	auto* softly = registry.AddComponent<SoftlyMoveComponent>(e);
	softly->amplitude.y = 64.f;
	registry.AddComponent<No::Velocity2DComponent>(e);
}

void RabbitdokuStageEditSystem::DeleteGimmick(No::Registry& registry) {
	No::Vector2 position = No::Get2DGameWindowMousePosition(registry);
	auto gimmickView = registry.View<GimmickTag, No::AABBCollider2D, No::Transform2DComponent>();
	for (auto e : gimmickView) {
		auto* aabb = registry.GetComponent<No::AABBCollider2D>(e);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		if (No::IsCollision(position, aabb, transform)) {
			registry.DestroyEntity(e);
		}
	}
}

void RabbitdokuStageEditSystem::DeleteBlock(No::Registry& registry) {
	No::Vector2 position = No::Get2DGameWindowMousePosition(registry);
	auto blockView = registry.View<BlockTag, No::AABBCollider2D, No::Transform2DComponent>();
	for (auto e : blockView) {
		auto* aabb = registry.GetComponent<No::AABBCollider2D>(e);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		if (No::IsCollision(position, aabb, transform)) {
			registry.DestroyEntity(e);
		}
	}
}

void RabbitdokuStageEditSystem::WarpPlayer(No::Registry& registry) {
	No::Vector2 position = No::Get2DGameWindowMousePosition(registry);
	auto view = registry.View<Rabbitdoku, No::Transform2DComponent>();
	for (auto e : view) {
		registry.GetComponent<No::Transform2DComponent>(e)->translate = position;
	}
}

No::Entity RabbitdokuStageEditSystem::FindRoom(No::Registry& registry, const No::Vector2& pos) {
	auto view = registry.View<RoomTag, No::Transform2DComponent>();
	for (auto e : view) {
		if (No::IsCollision(pos, registry.GetComponent<RoomTag>(e), registry.GetComponent<No::Transform2DComponent>(e))) return e;
	}
	return No::INVALID_ENTITY;
}

No::Entity RabbitdokuStageEditSystem::FindBackground(No::Registry& registry, const No::Vector2& pos) {
	auto view = registry.View<BackgroundTag, No::AABBCollider2D, No::Transform2DComponent>();
	for (auto e : view) {
		if (No::IsCollision(pos, registry.GetComponent<No::AABBCollider2D>(e), registry.GetComponent<No::Transform2DComponent>(e))) return e;
	}
	return No::INVALID_ENTITY;
}

No::Vector2 RabbitdokuStageEditSystem::GetGridPosition(const No::Vector2& position) {
	int cellX = (int)floor(position.x / gridSize_.x);
	int cellY = (int)floor(position.y / gridSize_.y);
	float centerX = cellX * gridSize_.x + gridSize_.x * 0.5f;
	float centerY = cellY * gridSize_.y + gridSize_.y * 0.5f;

	return { centerX, centerY };
}

void RabbitdokuStageEditSystem::DrawEditWindow(No::Registry& registry) {
#ifdef USE_IMGUI
	ImGui::Begin("StageEditor");
	ImGui::DragFloat2("GridSize", &gridSize_.x);

	if (ImGui::Button("ChangeMode")) {
		ImGui::OpenPopup("EditModePopup");
	}
	ImGui::SameLine();
	ImGui::Text("EditMode : ");
	ImGui::SameLine();
	switch (state_) {
	case RabbitdokuStageEditSystem::EditState::kBlock:
		ImGui::Text("BlockMode");
		break;
	case RabbitdokuStageEditSystem::EditState::kRoom:
		ImGui::Text("RoomMode");
		break;
	case RabbitdokuStageEditSystem::EditState::kGimmick:
		ImGui::Text("GimmickMode");
		break;
	case RabbitdokuStageEditSystem::EditState::kBackground:
		ImGui::Text("BackgroundMode");
		break;
	case RabbitdokuStageEditSystem::EditState::kPlayer:
		ImGui::Text("PlayerMode");
		break;
	default:
		ImGui::Text("UnknownMode");
		break;
	}

	if (ImGui::BeginPopupContextItem("EditModePopup")) {
		if (ImGui::MenuItem("BlockMode")) {
			state_ = EditState::kBlock;
		}
		if (ImGui::MenuItem("RoomMode")) {
			state_ = EditState::kRoom;
		}
		if (ImGui::MenuItem("GimmickMode")) {
			state_ = EditState::kGimmick;
		}
		if (ImGui::MenuItem("BackgroundMode")) {
			state_ = EditState::kBackground;
		}
		if (ImGui::MenuItem("PlayerMode")) {
			state_ = EditState::kPlayer;
		}
		ImGui::EndPopup();
	}

	switch (state_) {
	case RabbitdokuStageEditSystem::EditState::kBlock:
		if (ImGui::Button("Blue")) {
			currentBlockTexture = BlockTextures::kBlue;
		}
		ImGui::SameLine();
		if (ImGui::Button("ジェイムズ")) {
			currentBlockTexture = BlockTextures::kDaionkai;
		}
		break;
	case RabbitdokuStageEditSystem::EditState::kRoom:
		break;
	case RabbitdokuStageEditSystem::EditState::kGimmick:
		if (ImGui::Button("Save")) {
			gimmick_ = GimmickSelected::kSave;
		}
		ImGui::SameLine();
		if (ImGui::Button("DeathBlock")) {
			gimmick_ = GimmickSelected::kDeathBlock;
		}
		ImGui::SameLine();
		if (ImGui::Button("Needle")) {
			gimmick_ = GimmickSelected::kNeedle;
		}
		ImGui::SameLine();
		if (ImGui::Button("Spring")) {
			gimmick_ = GimmickSelected::kSpring;
		}
		ImGui::SameLine();
		if (ImGui::Button("CollapseBlock")) {
			gimmick_ = GimmickSelected::kCollapseBlock;
		}
		ImGui::SameLine();
		if (ImGui::Button("Replenisher")) {
			gimmick_ = GimmickSelected::kReplenisher;
		}
		ImGui::SameLine();
		if (ImGui::Button("Door")) {
			gimmick_ = GimmickSelected::kDoor;
		}
		ImGui::Text("Selected : ");
		ImGui::SameLine();
		switch (gimmick_) {
		case RabbitdokuStageEditSystem::GimmickSelected::kSave:
			ImGui::Text("Save");
			break;
		case RabbitdokuStageEditSystem::GimmickSelected::kDeathBlock:
			ImGui::Text("DeathBlock");
			break;
		case RabbitdokuStageEditSystem::GimmickSelected::kNeedle:
			ImGui::Text("Needle");
			break;
		case RabbitdokuStageEditSystem::GimmickSelected::kSpring:
			ImGui::Text("Spring");
			break;
		case RabbitdokuStageEditSystem::GimmickSelected::kCollapseBlock:
			ImGui::Text("CollapseBlock");
			break;
		case RabbitdokuStageEditSystem::GimmickSelected::kReplenisher:
			ImGui::Text("Replenisher");
			break;
		case RabbitdokuStageEditSystem::GimmickSelected::kDoor:
			ImGui::Text("Door");
			break;
		}
		break;
	case EditState::kBackground:
		if (ImGui::Button("ChangeTex")) {
			ImGui::OpenPopup("TexturePopup");
		}
		if (ImGui::BeginPopupContextItem("TexturePopup")) {
			if (ImGui::MenuItem("Flower")) {
				currentBackgroundTexture_ = BackgroundTextures::kFlower;
			}
			if (ImGui::MenuItem("Title")) {
				currentBackgroundTexture_ = BackgroundTextures::kTitle;
			}

			if (ImGui::MenuItem("Background01")) {
				currentBackgroundTexture_ = BackgroundTextures::kBackground01;
			}
			if (ImGui::MenuItem("Background02")) {
				currentBackgroundTexture_ = BackgroundTextures::kBackground02;
			}
			if (ImGui::MenuItem("Background03")) {
				currentBackgroundTexture_ = BackgroundTextures::kBackground03;
			}
			if (ImGui::MenuItem("Background04")) {
				currentBackgroundTexture_ = BackgroundTextures::kBackground04;
			}
			if (ImGui::MenuItem("Background05")) {
				currentBackgroundTexture_ = BackgroundTextures::kBackground05;
			}
			if (ImGui::MenuItem("Background06")) {
				currentBackgroundTexture_ = BackgroundTextures::kBackground06;
			}
			if (ImGui::MenuItem("Background07")) {
				currentBackgroundTexture_ = BackgroundTextures::kBackground07;
			}
			if (ImGui::MenuItem("Background08")) {
				currentBackgroundTexture_ = BackgroundTextures::kBackground08;
			}
			if (ImGui::MenuItem("Background09")) {
				currentBackgroundTexture_ = BackgroundTextures::kBackground09;
			}

			if (ImGui::MenuItem("StageNumber00")) {
				currentBackgroundTexture_ = BackgroundTextures::kStageNumber00;
			}
			if (ImGui::MenuItem("StageNumber01")) {
				currentBackgroundTexture_ = BackgroundTextures::kStageNumber01;
			}
			if (ImGui::MenuItem("StageNumber02")) {
				currentBackgroundTexture_ = BackgroundTextures::kStageNumber02;
			}
			if (ImGui::MenuItem("StageNumber03")) {
				currentBackgroundTexture_ = BackgroundTextures::kStageNumber03;
			}
			if (ImGui::MenuItem("StageNumber04")) {
				currentBackgroundTexture_ = BackgroundTextures::kStageNumber04;
			}
			if (ImGui::MenuItem("StageNumber05")) {
				currentBackgroundTexture_ = BackgroundTextures::kStageNumber05;
			}
			if (ImGui::MenuItem("StageNumber06")) {
				currentBackgroundTexture_ = BackgroundTextures::kStageNumber06;
			}
			if (ImGui::MenuItem("StageNumber07")) {
				currentBackgroundTexture_ = BackgroundTextures::kStageNumber07;
			}
			if (ImGui::MenuItem("StageNumber08")) {
				currentBackgroundTexture_ = BackgroundTextures::kStageNumber08;
			}
			if (ImGui::MenuItem("StageNumber09")) {
				currentBackgroundTexture_ = BackgroundTextures::kStageNumber09;
			}
			ImGui::EndPopup();
		}
		break;
	default:
		break;
	}

	ImGui::End();
#endif // USE_IMGUI
	static_cast<void>(registry);

}

void RabbitdokuStageEditSystem::DrawRooms(No::Registry& registry) {
#ifdef USE_IMGUI
	auto view = registry.View<RoomTag, No::Transform2DComponent>();
	for (auto e : view) {
		auto* box = registry.GetComponent<RoomTag>(e);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		NoEngine::DebugPrimitive::DrawCube2D(transform->translate, box->max, box->min, No::Color::BLUE);
	}
#else
	static_cast<void>(registry);
#endif // USE_IMGUI

}

void RabbitdokuStageEditSystem::DrawCollider(No::Registry& registry) {
	for (auto e : registry.View<No::Transform2DComponent, No::AABBCollider2D>()) {
		auto* c = registry.GetComponent<No::AABBCollider2D>(e);
		NoEngine::DebugPrimitive::DrawCube2D(registry.GetComponent<No::Transform2DComponent>(e)->translate,
			c->min, c->max, No::Color::RED);
	}
}
