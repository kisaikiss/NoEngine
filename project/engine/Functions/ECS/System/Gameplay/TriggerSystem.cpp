#include "TriggerSystem.h"
#include "engine/Functions/ECS/Event/CollisionEvents.h"
#include "engine/Functions/ECS/Event/GameplayEvents.h"
#include "engine/Functions/ECS/Component/Common/TriggerComponent.h"
#include "engine/Functions/ECS/Gameplay/GameplayCatalog.h"

namespace NoEngine::ECS {
void TriggerSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	// NarrowPhaseSystemが前段で発行した接触一覧を一度受け取り、Triggerごとに接触状態を調べる。
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
		// 前フレームは非接触で今フレーム接触した場合だけEnterを発行する。
		// oneShotなら最初のEnter後にactivatedを立て、以後は再発火させない。
		if (touching && !trigger->wasTouching && (!trigger->oneShot || !trigger->activated)) {
			Gameplay::EmitGameplayEvent(registry, triggerEntity, Event::GameplayId::TriggerEnter, instigator);
			if (trigger->oneShot) trigger->activated = true;
		}
		trigger->wasTouching = touching;
	}
	// PollEventで取り出した接触イベントを消費し切らないよう、後続システム用に再発行する。
	if (contacts) registry.EmitEvent(*contacts);
}
}

#include "../SystemCatalog.h"
REGISTER_SYSTEM(NoEngine::ECS::TriggerSystem, "TriggerSystem", "Gameplay")
