#include "stdafx.h"
#include "ColliderDrawSystem.h"
#include "../../Component/Option/EditorSettingComponent.h"

namespace {
// Collider表示では形状確認に必要な最低限の分割数に抑える。
constexpr uint32_t kColliderSphereSlices = 8;
constexpr uint32_t kColliderSphereStacks = 6;
}

void ColliderDrawSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	bool drawCollider = false;
	for (auto setting : registry.View<EditorSettingComponent>()) {
		drawCollider = registry.GetComponent<EditorSettingComponent>(setting)->drawCollider;
	}

	// 描画するフラグが立っていないなら早期リターン
	if (!drawCollider) return;

	for (auto capsuleE : registry.View<No::TransformComponent, No::CapsuleCollider>()) {

		auto* capsuleTransform = registry.GetComponent<No::TransformComponent>(capsuleE);
		auto* capsuleCollider = registry.GetComponent<No::CapsuleCollider>(capsuleE);
		const auto worldPosition = capsuleTransform->GetWorldPosition(registry);
		const auto p0 = worldPosition + capsuleCollider->localP0;
		const auto p1 = worldPosition + capsuleCollider->localP1;
		NoEngine::DebugPrimitive::DrawSphere(p0, capsuleCollider->radius, No::Color::WHITE, kColliderSphereSlices, kColliderSphereStacks);
		NoEngine::DebugPrimitive::DrawSphere(p1, capsuleCollider->radius, No::Color::WHITE, kColliderSphereSlices, kColliderSphereStacks);
		NoEngine::DebugPrimitive::DrawLine(p0, p1, No::Color::WHITE);

	}

	for (auto sphereE : registry.View<No::TransformComponent, No::SphereCollider>()) {
		auto* sphereTransform = registry.GetComponent<No::TransformComponent>(sphereE);
		auto* sphereCollider = registry.GetComponent<No::SphereCollider>(sphereE);
		NoEngine::DebugPrimitive::DrawSphere(sphereTransform->GetWorldPosition(registry) + sphereCollider->localCenter, sphereCollider->radius, No::Color::WHITE, kColliderSphereSlices, kColliderSphereStacks);
	}

	for (auto boxE : registry.View<No::TransformComponent, No::AABBCollider>()) {
		auto* boxTransform = registry.GetComponent<No::TransformComponent>(boxE);
		auto* boxCollider = registry.GetComponent<No::AABBCollider>(boxE);
		No::Vector3 size = No::Vector3(
			(std::fabsf(boxCollider->min.x) + std::fabsf(boxCollider->max.x)),
			(std::fabsf(boxCollider->min.y) + std::fabsf(boxCollider->max.y)),
			(std::fabsf(boxCollider->min.z) + std::fabsf(boxCollider->max.z))
		);
		NoEngine::DebugPrimitive::DrawCube(boxTransform->GetWorldPosition(registry), size, No::Color::WHITE);
	}

	for (auto boxE : registry.View<No::TransformComponent, No::OBBCollider>()) {
		auto* boxTransform = registry.GetComponent<No::TransformComponent>(boxE);
		auto* boxCollider = registry.GetComponent<No::OBBCollider>(boxE);
		NoEngine::DebugPrimitive::DrawCube(boxTransform->GetWorldPosition(registry), boxCollider->extents, boxTransform->rotation, No::Color::WHITE);
	}
}

REGISTER_SYSTEM(::ColliderDrawSystem, "ColliderDrawSystem", "Gameplay")
