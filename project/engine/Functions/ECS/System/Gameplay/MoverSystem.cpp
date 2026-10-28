#include "MoverSystem.h"
#include "engine/Functions/ECS/Event/GameplayEvents.h"
#include "engine/Functions/ECS/Component/Common/MoverComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformRoutineComponent.h"

namespace NoEngine::ECS {
void MoverSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	// 初期停止状態を先に適用し、その後の起動イベントを同フレームに反映する。
	for (auto entity : registry.View<Component::MoverComponent, Component::TransformRoutineComponent>()) {
		auto* mover = registry.GetComponent<Component::MoverComponent>(entity);
		auto* routine = registry.GetComponent<Component::TransformRoutineComponent>(entity);
		if (mover->initialized) continue;
		mover->initialized = true;
		mover->active = mover->startOnAwake;
		routine->playing = mover->startOnAwake;
	}

	for (const auto& event : registry.PollAllEvents<Event::ActivateMoverEvent>()) {
		auto* mover = registry.GetComponent<Component::MoverComponent>(event.targetEntity);
		auto* routine = registry.GetComponent<Component::TransformRoutineComponent>(event.targetEntity);
		if (!mover || !routine) continue;
		mover->active = true;
		routine->currentIndex = 0;
		routine->elapsed = 0.0f;
		routine->playing = true;
	}
}
}

#include "../SystemCatalog.h"
REGISTER_SYSTEM(NoEngine::ECS::MoverSystem, "MoverSystem", "Gameplay")
