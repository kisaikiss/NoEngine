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

}
