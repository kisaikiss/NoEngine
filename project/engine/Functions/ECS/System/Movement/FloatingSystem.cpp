#include "stdafx.h"
#include "FloatingSystem.h"
#include "../../Component/Common/TransformComponent.h"
#include "../../Component/Common/FloatingComponent.h"
#include "../../Component/Common/TransformRoutineComponent.h"
#include "../SystemManager.h"

using namespace NoEngine;

void NoEngine::ECS::FloatingSystem::Update(Registry& registry, float deltaTime) {
	for (auto e : registry.View<Component::TransformComponent, Component::FloatingComponent>()) {
		auto* transform = registry.GetComponent<Component::TransformComponent>(e);
		auto* floating = registry.GetComponent<Component::FloatingComponent>(e);
		if (!transform || !floating) {
			continue;
		}

		// TransformRoutineSystemは、毎フレーム、絶対的なキーフレーム位置を書き込みます。
		// そのため、現在の位置には、以前のオフセットは含まれていません。
		const auto* routine = registry.GetComponent<Component::TransformRoutineComponent>(e);
		const bool routineIsDrivingPosition = routine && routine->playing && routine->keyframes.size() >= 2;
		if (!routineIsDrivingPosition) {
			transform->translate -= floating->previousOffset;
		}

		floating->elapsedTime += deltaTime;
		const float time = floating->elapsedTime;
		floating->previousOffset = Math::Vector3{
			floating->amplitude.x * std::sin(time * floating->velocity.x),
			floating->amplitude.y * std::sin(time * floating->velocity.y),
			floating->amplitude.z * std::sin(time * floating->velocity.z)};
		transform->translate += floating->previousOffset;

	}
}

REGISTER_SYSTEM(NoEngine::ECS::FloatingSystem, "FloatingSystem", "Movement")
