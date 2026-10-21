#include "stdafx.h"
#include "PlatformRideSystem.h"
#include "../../Component/Player/PlayerComponent.h"

void PlatformRideSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	for (auto e : registry.View<PlayerComponent, No::TransformComponent, No::GroundStateComponent>()) {
		auto* ground = registry.GetComponent<No::GroundStateComponent>(e);
		if (!ground->isGrounded || ground->groundEntity == No::INVALID_ENTITY) continue;

		auto* delta = registry.GetComponent<No::PlatformDeltaComponent>(ground->groundEntity);
		if (!delta) continue;

		auto* transform = registry.GetComponent<No::TransformComponent>(e);
		const No::Vector3 playerPos = transform->GetWorldPosition(registry);

		// 前フレームの足場から見た相対位置 → 現在の足場へ置き直す
		No::Quaternion invPrev = delta->previousRotation;
		invPrev.Normalize();
		invPrev.Inverse();
		const No::Vector3 rel = invPrev.RotateVector(playerPos - delta->previousTranslate);

		No::Quaternion curRot = delta->currentRotation;
		curRot.Normalize();
		const No::Vector3 newPos = delta->currentTranslate + curRot.RotateVector(rel);

		transform->SetWorldPosition(registry, newPos);

		// (任意) 足場のY回転量だけプレイヤーの向きも回す
		No::Quaternion deltaRot = curRot * invPrev;
		No::Vector3 f = deltaRot.RotateVector(No::Vector3::FORWARD);
		if (std::fabsf(f.x) + std::fabsf(f.z) > 1e-6f) {
			No::Quaternion yaw;
			yaw.FromAxisAngle(No::Vector3::UP, std::atan2f(f.x, f.z));
			transform->rotation = yaw * transform->rotation;
		}
	}
}

REGISTER_SYSTEM(::PlatformRideSystem, "PlatformRideSystem", "Gameplay")
