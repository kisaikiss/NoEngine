#include "EventLinkSystem.h"
#include "engine/Functions/ECS/Event/GameplayEvents.h"
#include "engine/Functions/ECS/Component/Common/EventLinkComponent.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"

namespace NoEngine::ECS {
void EventLinkSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	for (const auto& event : registry.PollAllEvents<Event::GameplayEvent>()) {
		auto* links = registry.GetComponent<Component::EventLinkComponent>(event.source);
		if (!links) continue;

		for (const auto& link : links->links) {
			if (link.eventName != event.eventName || link.targetEntity == ECS::INVALID_ENTITY) continue;

			// 現状はActivateMoverEventのみなのでここで発行している
			// いずれはEventLinkComponentで指定して汎用的にイベントを発行できるようにする
			registry.EmitEvent(Event::ActivateMoverEvent{ link.targetEntity });
		}
	}
}
}

#include "../SystemCatalog.h"
REGISTER_SYSTEM(NoEngine::ECS::EventLinkSystem, "EventLinkSystem", "Gameplay")
