#include "stdafx.h"
#include "DialogueAutoTriggerSystem.h"
#include "../../Component/Dialogue/DialogueComponent.h"
#include "../../Component/Player/PlayerComponent.h"

void DialogueAutoTriggerSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	for (auto player : registry.View<PlayerComponent, No::TransformComponent>()) {
		if (registry.Has<DialogueLockTag>(player)) continue;
		No::Vector3 playerPos = registry.GetComponent<No::TransformComponent>(player)->GetWorldPosition(registry);

		for (auto e : registry.View<DialogueAutoTriggerComponent, DialogueComponent, No::TransformComponent>()) {
			auto* trigger = registry.GetComponent<DialogueAutoTriggerComponent>(e);
			if (trigger->once && trigger->triggered) continue;

			float dist = (registry.GetComponent<No::TransformComponent>(e)->GetWorldPosition(registry) - playerPos).Length();
			if (dist <= trigger->radius) {
				trigger->triggered = true;
				registry.EmitEvent(DialogueStartEvent{ e });
				return; // 1フレームに1会話だけ
			}
		}
	}
}
REGISTER_SYSTEM(::DialogueAutoTriggerSystem, "DialogueAutoTriggerSystem", "Gameplay")