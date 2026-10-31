#include "MoverSystem.h"
#include "engine/Functions/ECS/Event/GameplayEvents.h"
#include "engine/Functions/ECS/Component/Common/MoverComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformRoutineComponent.h"

namespace NoEngine::ECS {
void MoverSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	// シーン読込後の初回だけMoverの開始状態をTransformRoutineへ反映する。
	for (auto entity : registry.View<Component::MoverComponent, Component::TransformRoutineComponent>()) {
		auto* mover = registry.GetComponent<Component::MoverComponent>(entity);
		auto* routine = registry.GetComponent<Component::TransformRoutineComponent>(entity);
		if (mover->initialized) continue;
		mover->initialized = true;
		mover->active = mover->startOnAwake;
		routine->playing = mover->startOnAwake;
	}

	// GameplayCatalogのMover Actionが出した内部イベントを受けて、ルートを先頭から再生する。
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
