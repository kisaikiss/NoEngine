#include "PlayerInputSystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"

void PlayerInputSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	for (auto entity : registry.View<PlayerInputComponent>()) {
		auto* input = registry.GetComponent<PlayerInputComponent>(entity);
		if (!input) continue;

		input->moveAxis.x = No::GetInputAxisValue("Lateral");
		input->moveAxis.y = 0.0f;
		input->moveAxis.z = No::GetInputAxisValue("Forward");
		input->jumpTriggered = No::InputIsTrigger("Jump");
		input->jumpReleased = No::InputIsRelease("Jump");
		input->highJumpTriggered = No::InputIsTrigger("HighJump");
		input->highJumpPressed = No::InputIsPress("HighJump");
		input->highJumpReleased = No::InputIsRelease("HighJump");
		input->airDashPressed = No::InputIsPress("AirDash");
		input->airDashReleased = No::InputIsRelease("AirDash");
		input->createScaffoldTriggered = No::InputIsTrigger("CreateScaffold");
	}
}

REGISTER_SYSTEM(::PlayerInputSystem, "PlayerInputSystem", "Gameplay")
