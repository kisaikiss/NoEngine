#include "ClockworksDisease.h"
#include "Scene/TestScene.h"

void ClockworksDisease::Startup(void) {
	RegisterScene("TestScene", []() { return std::make_unique<TestScene>();	});
	ChangeScene("TestScene");

	No::InputBindAxis("Horizontal", No::DeviceType::GamepadAxis, static_cast<int>(No::GamepadAxis::LeftStickX), 1.0f, 0.2f);
	No::InputBindAxis("Horizontal", No::DeviceType::Keyboard, static_cast<int>('D'), 1.0f);
	No::InputBindAxis("Horizontal", No::DeviceType::Keyboard, static_cast<int>('A'), -1.0f);

	No::InputBindAxis("Forward", No::DeviceType::GamepadAxis, static_cast<int>(No::GamepadAxis::LeftStickY), 1.0f, 0.2f);
	No::InputBindAxis("Forward", No::DeviceType::Keyboard, static_cast<int>('W'), 1.0f);
	No::InputBindAxis("Forward", No::DeviceType::Keyboard, static_cast<int>('S'), -1.0f);
}
