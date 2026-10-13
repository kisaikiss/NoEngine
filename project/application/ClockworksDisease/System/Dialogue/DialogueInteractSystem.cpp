#include "stdafx.h"
#include "DialogueInteractSystem.h"
#include "../../Component/Dialogue/DialogueComponent.h"
#include "../../Component/Player/PlayerComponent.h"

void DialogueInteractSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	if (!No::InputIsTrigger("Talk")) return;

	for (auto player : registry.View<PlayerComponent, No::TransformComponent>()) {
		if (registry.Has<DialogueLockTag>(player)) continue; // 会話中は新規に始めない
		No::Vector3 playerPos = registry.GetComponent<No::TransformComponent>(player)->GetWorldPosition(registry);

		// 範囲内で最も近い話し相手を選ぶ
		No::Entity nearest = No::INVALID_ENTITY;
		float best = 1e30f;
		for (auto e : registry.View<DialogueComponent, No::TransformComponent>()) {
			auto* dialogue = registry.GetComponent<DialogueComponent>(e);
			if (!dialogue->interactable || dialogue->lines.empty()) continue;

			float dist = (registry.GetComponent<No::TransformComponent>(e)->GetWorldPosition(registry) - playerPos).Length();
			if (dist <= dialogue->interactDistance && dist < best) {
				best = dist;
				nearest = e;
			}
		}
		if (nearest != No::INVALID_ENTITY) {
			registry.EmitEvent(DialogueStartEvent{ nearest });
		}
	}
}

REGISTER_SYSTEM(::DialogueInteractSystem, "DialogueInteractSystem", "Gameplay")