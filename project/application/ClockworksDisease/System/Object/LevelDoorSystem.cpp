#include "stdafx.h"
#include "LevelDoorSystem.h"
#include "../../Component/Game/LevelDoorComponent.h"
#include "../../Component/Player/PlayerComponent.h"
#include "../../Component/Stage/StageComponent.h"

namespace {
constexpr float kDigitUvWidth = 0.1f;
constexpr float kDigitLocalSpacing = 0.65f;
constexpr No::Vector3 kDigitScale = No::Vector3(0.3f, 0.15f, 1.0f);

void UpdateLevelLabel(No::Registry& registry, No::Entity doorEntity, LevelDoorComponent& door) {
	const std::string digits = std::to_string(door.requiredLevel);
	while (door.levelDigits.size() < digits.size()) {
		const No::Entity digitEntity = registry.GenerateEntity();
		auto* mesh = registry.AddComponent<No::MeshComponent>(digitEntity);
		mesh->meshName = "resources/engine/Model/test/plane.obj";
		auto* material = registry.AddComponent<No::MaterialComponent>(digitEntity);
		material->textureName = "levelNumbers";
		material->unlit = true;
		material->uvScale = { kDigitUvWidth, 1.0f };
		material->blendMode = NoEngine::BlendMode::kAlpha;
		auto* transform = registry.AddComponent<No::TransformComponent>(digitEntity);
		transform->parent = doorEntity;
		transform->scale = kDigitScale;
		door.levelDigits.push_back(digitEntity);
	}

	const No::Vector3& offset = door.digitsOffset;
	for (size_t i = 0; i < door.levelDigits.size(); ++i) {
		const No::Entity digitEntity = door.levelDigits[i];
		auto* mesh = registry.GetComponent<No::MeshComponent>(digitEntity);
		auto* material = registry.GetComponent<No::MaterialComponent>(digitEntity);
		auto* transform = registry.GetComponent<No::TransformComponent>(digitEntity);
		if (!mesh || !material || !transform) continue;
		if (i >= digits.size()) {
			mesh->isVisible = false;
			continue;
		}

		mesh->isVisible = !door.isOpening;
		material->uvPosition = { static_cast<float>(digits[i] - '0') * kDigitUvWidth, 0.0f };
		const size_t displayIndex = digits.size() - 1 - i;
		transform->translate = offset;
		transform->translate.x += (static_cast<float>(displayIndex) - (static_cast<float>(digits.size()) - 1.0f) * 0.5f) * kDigitLocalSpacing;
	}
}

void DestroyLevelLabel(No::Registry& registry, LevelDoorComponent& door) {
	for (const No::Entity digit : door.levelDigits) {
		if (registry.GetComponent<No::MeshComponent>(digit)) registry.DestroyEntity(digit);
	}
	door.levelDigits.clear();
}
}

void LevelDoorSystem::Update(No::Registry& registry, float deltaTime) {
	for (const No::Entity doorEntity : registry.View<LevelDoorComponent, No::TransformComponent, No::MeshComponent>()) {
		auto* door = registry.GetComponent<LevelDoorComponent>(doorEntity);
		auto* transform = registry.GetComponent<No::TransformComponent>(doorEntity);
		const No::Vector3 doorPosition = transform->GetWorldPosition(registry);
		if (door->isOpening) {
			door->animationTimer += deltaTime;
			const float progress = std::clamp(door->animationTimer / std::max(door->animationDuration, 0.01f), 0.0f, 1.0f);
			const float eased = No::ApplyEasing(No::EasingType::EaseInBack, progress);
			transform->scale = No::Lerp(No::Vector3::UNIT_SCALE, No::Vector3::ZERO, eased);
			No::Vector3 openingPosition = door->openingStartPosition;
			constexpr float magnification = 0.6f;
			openingPosition.y += progress * magnification;
			transform->SetWorldPosition(registry, openingPosition);
			if (progress >= 1.0f) {
				DestroyLevelLabel(registry, *door);
				registry.DestroyEntity(doorEntity);
			}
			continue;
		}

		UpdateLevelLabel(registry, doorEntity, *door);
		for (const No::Entity playerEntity : registry.View<PlayerComponent, LevelComponent, No::TransformComponent>()) {
			const auto* level = registry.GetComponent<LevelComponent>(playerEntity);
			const No::Vector3 playerPosition = registry.GetComponent<No::TransformComponent>(playerEntity)->GetWorldPosition(registry);
			if (level->nowLevel <= door->requiredLevel || (playerPosition - doorPosition).Length() > door->activationDistance) continue;

			door->isOpening = true;
			door->animationTimer = 0.0f;
			door->openingStartPosition = doorPosition;
			for (const No::Entity digit : door->levelDigits) {
				if (auto* mesh = registry.GetComponent<No::MeshComponent>(digit)) mesh->isVisible = false;
			}
			if (registry.Has<No::AABBCollider>(doorEntity)) registry.RemoveComponent<No::AABBCollider>(doorEntity);
			if (registry.Has<No::SphereCollider>(doorEntity)) registry.RemoveComponent<No::SphereCollider>(doorEntity);
			if (registry.Has<ColliderUpdateTag>(doorEntity)) registry.RemoveComponent<ColliderUpdateTag>(doorEntity);
			break;
		}
	}
}

REGISTER_SYSTEM(::LevelDoorSystem, "LevelDoorSystem", "Gameplay")
