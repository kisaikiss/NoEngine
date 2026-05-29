#include "stdafx.h"
#include "RabbitdokuOdyssey3Plus.h"
#include "Scene/RabbitdokuScene.h"
#include "Scene/RabbitdokuTitleScene.h"
#include "Scene/ExxsearearStage.h"

void RabbitdokuOdyssey3Plus::Startup(void) {
	RegisterScene("Rabbitdoku", []() { return std::make_unique<RabbitdokuScene>();	});
	RegisterScene("ExxsearearStage", []() { return std::make_unique<ExxsearearStage>();	});
	RegisterScene("Title", []() { return std::make_unique<RabbitdokuTitleScene>(); });
	ChangeScene("Title");

	auto* roomBox = NoEngine::ComponentRegistry::FindByName("RoomTag");
	roomBox->fields[0].attributes.valueSpeed = 1.f;
	roomBox->fields[1].attributes.valueSpeed = 1.f;

	No::InputBindAction("Jump", No::DeviceType::Keyboard, static_cast<int>(VK_LSHIFT));
	No::InputBindAction("Jump", No::DeviceType::Keyboard, static_cast<int>(VK_RSHIFT));
	No::InputBindAction("Jump", No::DeviceType::Gamepad, static_cast<int>(No::GamepadButton::A));

	No::InputBindAction("Right", No::DeviceType::Keyboard, static_cast<int>(VK_RIGHT));
	No::InputBindAction("Right", No::DeviceType::Gamepad, static_cast<int>(No::GamepadButton::Right));
	No::InputBindAction("Left", No::DeviceType::Keyboard, static_cast<int>(VK_LEFT));
	No::InputBindAction("Left", No::DeviceType::Gamepad, static_cast<int>(No::GamepadButton::Left));

	No::InputBindAction("Reset", No::DeviceType::Keyboard, static_cast<int>('R'));
	No::InputBindAction("Reset", No::DeviceType::Gamepad, static_cast<int>(No::GamepadButton::Y));

	No::InputBindAction("Save", No::DeviceType::Keyboard, static_cast<int>('Z'));
	No::InputBindAction("Save", No::DeviceType::Gamepad, static_cast<int>(No::GamepadButton::X));
}
