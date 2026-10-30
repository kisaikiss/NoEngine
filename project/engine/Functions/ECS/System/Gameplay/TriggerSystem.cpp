#include "TriggerSystem.h"
#include "engine/Functions/ECS/Event/CollisionEvents.h"
#include "engine/Functions/ECS/Event/GameplayEvents.h"
#include "engine/Functions/ECS/Component/Common/TriggerComponent.h"
#include "engine/Functions/ECS/Gameplay/GameplayCatalog.h"

namespace NoEngine::ECS {
void TriggerSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto contacts = registry.PollEvent<Event::ContactEvent>();
	for (auto triggerEntity : registry.View<Component::TriggerComponent>()) {
		auto* trigger = registry.GetComponent<Component::TriggerComponent>(triggerEntity);
		bool touching = false;
		Entity instigator = INVALID_ENTITY;
		if (contacts) {
			for (const auto& contact : contacts->contacts) {
				if (contact.a == triggerEntity) {
					instigator = contact.b;
					touching = true;
					break;
				} else if (contact.b == triggerEntity) {
					instigator = contact.a;
					touching = true;
					break;
				}
			}
		}
		if (touching && !trigger->wasTouching && (!trigger->oneShot || !trigger->activated)) {
			Gameplay::EmitGameplayEvent(registry, triggerEntity, Event::GameplayId::TriggerEnter, instigator);
			if (trigger->oneShot) trigger->activated = true;
		}
		trigger->wasTouching = touching;
	}
	// ContactEventは他のゲームシステムにも渡す。
	if (contacts) registry.EmitEvent(*contacts);
}
}

#include "../SystemCatalog.h"
REGISTER_SYSTEM(NoEngine::ECS::TriggerSystem, "TriggerSystem", "Gameplay")
