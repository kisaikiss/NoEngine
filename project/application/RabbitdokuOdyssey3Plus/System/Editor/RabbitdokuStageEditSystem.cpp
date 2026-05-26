#include "stdafx.h"
#include "RabbitdokuStageEditSystem.h"
#include "application/RabbitdokuOdyssey3Plus/Game/RabbitdokuCollisionLayer.h"
#include "application/RabbitdokuOdyssey3Plus/Game/RabbitdokuTag.h"

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
		if (No::Mouse::IsTrigger(No::MouseButton::Left)) {
			if (No::IsMouseOverGameWindow()) {
				switch (gimmick_) {
				case RabbitdokuStageEditSystem::GimmickSlected::kSave:
					AddSave(registry);
					break;
				case RabbitdokuStageEditSystem::GimmickSlected::kDeathBlock:
					AddNeedle(registry);
					break;
				case RabbitdokuStageEditSystem::GimmickSlected::kNeedle:
					break;
				default:
					break;
				}
			}
		}

		if (No::Mouse::IsTrigger(No::MouseButton::Right)) {
			if (No::IsMouseOverGameWindow())
				DeleteGimmick(registry);
		}
		break;
	}


	No::DrawGrid2D(gridSize_);

	DrawRooms(registry);
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
	auto* collider = registry.AddComponent<No::AABBCollider2D>(e);
	registry.AddComponent<BlockTag>(e);
	auto* tag = registry.AddComponent<No::EditTag>(e);
	tag->name = "block";
	tag->path = "Block/block";
	registry.AddComponent<No::CollisionBody>(e)->type = No::BodyType::Static;
	registry.AddComponent<RabbitdokuCollisionLayerComponent>(e)->layer = RabbitdokuCollisionLayerComponent::Terrain;

	sprite->textureFilePath = "resources/game/RabbitdokuOdyssey3Plus/Sprite/block.png";
	transform->scale.x = gridSize_.x;
	transform->scale.y = gridSize_.y;
	collider->max = transform->scale / 2.f;
	collider->min = -transform->scale / 2.f;
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
	sprite->uv.width = 1.f/8.f;
	sprite->uv.height = 1.f;

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
		auto* aabb = registry.GetComponent<No::AABBCollider2D>(e);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		if (No::IsCollision(position, aabb, transform)) return;
	}


	No::Entity e = registry.GenerateEntity();
	auto* transform = registry.AddComponent<No::Transform2DComponent>(e);
	transform->translate = position;
	auto* sprite = registry.AddComponent<No::SpriteComponent>(e);
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

No::Entity RabbitdokuStageEditSystem::FindRoom(No::Registry& registry, const No::Vector2& pos) {
	auto view = registry.View<RoomTag, No::Transform2DComponent>();
	for (auto e : view) {
		if (No::IsCollision(pos, registry.GetComponent<RoomTag>(e), registry.GetComponent<No::Transform2DComponent>(e))) return e;
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

		ImGui::EndPopup();
	}

	switch (state_) {
	case RabbitdokuStageEditSystem::EditState::kBlock:
		break;
	case RabbitdokuStageEditSystem::EditState::kRoom:
		break;
	case RabbitdokuStageEditSystem::EditState::kGimmick:
		if (ImGui::Button("Save")) {
			gimmick_ = GimmickSlected::kSave;
		}
		ImGui::SameLine();
		if (ImGui::Button("DeathBlock")) {
			gimmick_ = GimmickSlected::kDeathBlock;
		}
		ImGui::Text("Selected : ");
		ImGui::SameLine();
		switch (gimmick_) {
		case RabbitdokuStageEditSystem::GimmickSlected::kSave:
			ImGui::Text("Save");
			break;
		case RabbitdokuStageEditSystem::GimmickSlected::kDeathBlock:
			ImGui::Text("DeathBlock");
			break;
		case RabbitdokuStageEditSystem::GimmickSlected::kNeedle:
			ImGui::Text("Needle");
			break;
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
		NoEngine::DebugPrimitive::DrawCube2D(transform->translate, box->max, box->min, No::Color::RED);
	}
#else
	static_cast<void>(registry);
#endif // USE_IMGUI

}
