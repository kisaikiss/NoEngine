#include "stdafx.h"
#include "ColliderDrawSystem.h"
#include "../../Component/Option/EditorSettingComponent.h"

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
		NoEngine::DebugPrimitive::DrawSphere(capsuleTransform->GetWorldPosition(registry) + capsuleCollider->localP0, capsuleCollider->radius, No::Color::WHITE);
		NoEngine::DebugPrimitive::DrawSphere(capsuleTransform->GetWorldPosition(registry) + capsuleCollider->localP1, capsuleCollider->radius, No::Color::WHITE);
		NoEngine::DebugPrimitive::DrawLine(capsuleTransform->GetWorldPosition(registry) + capsuleCollider->localP0, capsuleTransform->GetWorldPosition(registry) + capsuleCollider->localP1, No::Color::WHITE);

	}

	for (auto sphereE : registry.View<No::TransformComponent, No::SphereCollider>()) {
		auto* sphereTransform = registry.GetComponent<No::TransformComponent>(sphereE);
		auto* sphereCollider = registry.GetComponent<No::SphereCollider>(sphereE);
		NoEngine::DebugPrimitive::DrawSphere(sphereTransform->GetWorldPosition(registry) + sphereCollider->localCenter, sphereCollider->radius, No::Color::WHITE);
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
}
