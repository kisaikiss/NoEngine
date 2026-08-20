#include "stdafx.h"
#include "PlayerAbilityDebugSystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerMoveTags.h"

void PlayerAbilityDebugSystem::Update(No::Registry& registry, float deltaTime) {
#ifdef USE_IMGUI
	for (auto e : registry.View<PlayerAbilityDebugComponent>()) {
		auto* debug = registry.GetComponent<PlayerAbilityDebugComponent>(e);

		if (debug->airDash) {
			if (!registry.Has<AirDashTag>(e)) {
				registry.AddComponent<AirDashTag>(e);
			}
		} else {
			if (registry.Has<AirDashTag>(e)) {
				registry.RemoveComponent<AirDashTag>(e);
			}
		}
		if (debug->highJump) {
			if (!registry.Has<HighJumpTag>(e)) {
				registry.AddComponent<HighJumpTag>(e);
			}
		} else {
			if (registry.Has<HighJumpTag>(e)) {
				registry.RemoveComponent<HighJumpTag>(e);
			}
		}
		if (debug->magicScaffold) {
			if (!registry.Has<CreateMagicScaffoldTag>(e)) {
				registry.AddComponent<CreateMagicScaffoldTag>(e);
			}
		} else {
			if (registry.Has<CreateMagicScaffoldTag>(e)) {
				registry.RemoveComponent<CreateMagicScaffoldTag>(e);
			}
		}

	}

#else
	static_cast<void>(registry);
#endif // USE_IMGUI
	static_cast<void>(deltaTime);
}
