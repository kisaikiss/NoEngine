#include "EventLinkSystem.h"
#include "engine/Functions/ECS/Event/GameplayEvents.h"
#include "engine/Functions/ECS/Component/Common/EventLinkComponent.h"
#include "engine/Functions/ECS/Gameplay/GameplayCatalog.h"

namespace NoEngine::ECS {
void EventLinkSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	for (const auto& event : registry.PollAllEvents<Event::GameplayEvent>()) {
		auto* links = registry.GetComponent<Component::EventLinkComponent>(event.source);
		if (!links) continue;

		for (const auto& link : links->links) {
			if (link.eventId != event.eventId) continue;
			auto* action = Gameplay::GameplayCatalog::Get().FindAction(link.actionId);
			if (!action) {
				LogWarning("EventLinkSystem: unregistered action id: " + link.actionId);
				continue;
			}
			if (action->requiresTarget && link.targetEntity == ECS::INVALID_ENTITY) {
				LogWarning("EventLinkSystem: action requires a target entity: " + link.actionId);
				continue;
			}
			action->execute(registry, event, link.targetEntity);
		}
	}
}
}

#include "../SystemCatalog.h"
REGISTER_SYSTEM(NoEngine::ECS::EventLinkSystem, "EventLinkSystem", "Gameplay")
