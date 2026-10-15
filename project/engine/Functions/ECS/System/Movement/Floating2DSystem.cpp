#include "stdafx.h"
#include "Floating2DSystem.h"
#include "../../Component/Common/Transform2DComponent.h"
#include "../../Component/Common/FloatingComponent.h"
#include "../../Component/Common/TransformRoutineComponent2D.h"
#include "../SystemManager.h"

using namespace NoEngine;

void NoEngine::ECS::Floating2DSystem::Update(Registry& registry, float deltaTime) {
	for (auto e : registry.View<Component::Transform2DComponent, Component::Floating2DComponent>()) {
		auto* transform = registry.GetComponent<Component::Transform2DComponent>(e);
		auto* floating = registry.GetComponent<Component::Floating2DComponent>(e);
		if (!transform || !floating) {
			continue;
		}

		// TransformRoutineSystemは、毎フレーム、絶対的なキーフレーム位置を書き込みます。
		// そのため、現在の位置には、以前のオフセットは含まれていません。
		const auto* routine = registry.GetComponent<Component::TransformRoutineComponent2D>(e);
		const bool routineIsDrivingPosition = routine && routine->playing && routine->keyframes.size() >= 2;
		if (!routineIsDrivingPosition) {
			transform->translate -= floating->previousOffset;
		}

		floating->elapsedTime += deltaTime;
		const float time = floating->elapsedTime;
		floating->previousOffset = Math::Vector2{
			floating->amplitude.x * std::sin(time * floating->velocity.x),
			floating->amplitude.y * std::sin(time * floating->velocity.y) };
		transform->translate += floating->previousOffset;

	}
}

REGISTER_SYSTEM(NoEngine::ECS::Floating2DSystem, "Floating2DSystem", "Movement")
