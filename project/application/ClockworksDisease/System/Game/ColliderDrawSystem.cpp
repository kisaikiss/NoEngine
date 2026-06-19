#include "stdafx.h"
#include "ColliderDrawSystem.h"

void ColliderDrawSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	for (auto capsuleE : registry.View<No::TransformComponent, No::CapsuleCollider>()) {

		auto* capsuleTransform = registry.GetComponent<No::TransformComponent>(capsuleE);
		auto* capsuleCollider = registry.GetComponent<No::CapsuleCollider>(capsuleE);
		NoEngine::DebugPrimitive::DrawSphere(capsuleTransform->GetWorldPosition() + capsuleCollider->localP0, capsuleCollider->radius, No::Color::WHITE);
		NoEngine::DebugPrimitive::DrawSphere(capsuleTransform->GetWorldPosition() + capsuleCollider->localP1, capsuleCollider->radius, No::Color::WHITE);
		NoEngine::DebugPrimitive::DrawLine(capsuleTransform->GetWorldPosition() + capsuleCollider->localP0, capsuleTransform->GetWorldPosition() + capsuleCollider->localP1, No::Color::WHITE);

	}

	for (auto sphereE : registry.View<No::TransformComponent, No::SphereCollider>()) {
		auto* sphereTransform = registry.GetComponent<No::TransformComponent>(sphereE);
		auto* sphereCollider = registry.GetComponent<No::SphereCollider>(sphereE);
		NoEngine::DebugPrimitive::DrawSphere(sphereTransform->GetWorldPosition() + sphereCollider->localCenter, sphereCollider->radius, No::Color::WHITE);
	}

}
