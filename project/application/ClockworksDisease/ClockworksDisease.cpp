#include "ClockworksDisease.h"
#include "Scene/GameScene.h"
#include "Scene/TitleScene.h"
#include "Scene/GameClearScene.h"

void ClockworksDisease::Startup(void) {
	RegisterScene("TestScene", []() { return std::make_unique<GameScene>();	});
	RegisterScene("SampleScene", []() { return std::make_unique<GameScene>();	});

	RegisterScene("GameScene", []() { return std::make_unique<GameScene>();	});
	RegisterScene("TitleScene", []() { return std::make_unique<TitleScene>();	});
	RegisterScene("GameClearScene", []() {return std::make_unique<GameClearScene>(); });
	ChangeScene("SampleScene");

	No::InputBindAxis("Lateral", No::DeviceType::GamepadAxis, static_cast<int>(No::GamepadAxis::LeftStickX), 1.0f, 0.2f);
	No::InputBindAxis("Lateral", No::DeviceType::Keyboard, static_cast<int>('D'), 1.0f);
	No::InputBindAxis("Lateral", No::DeviceType::Keyboard, static_cast<int>('A'), -1.0f);

	No::InputBindAxis("Forward", No::DeviceType::GamepadAxis, static_cast<int>(No::GamepadAxis::LeftStickY), 1.0f, 0.2f);
	No::InputBindAxis("Forward", No::DeviceType::Keyboard, static_cast<int>('W'), 1.0f);
	No::InputBindAxis("Forward", No::DeviceType::Keyboard, static_cast<int>('S'), -1.0f);

	No::InputBindAction("Jump", No::DeviceType::Keyboard, static_cast<int>(VK_SPACE));
	No::InputBindAction("Jump", No::DeviceType::GamepadButton, static_cast<int>(No::GamepadButton::A));

	No::InputBindAction("HighJump", No::DeviceType::Keyboard, static_cast<int>('E'));
	No::InputBindAction("HighJump", No::DeviceType::GamepadButton, static_cast<int>(No::GamepadButton::B));

	No::InputBindAxis("CameraHorizontal", No::DeviceType::GamepadAxis, static_cast<int>(No::GamepadAxis::RightStickX), -1.0f, 0.2f);
	No::InputBindAxis("CameraHorizontal", No::DeviceType::Keyboard, static_cast<int>(VK_RIGHT), -1.0f);
	No::InputBindAxis("CameraHorizontal", No::DeviceType::Keyboard, static_cast<int>(VK_LEFT), 1.0f);

	No::InputBindAxis("CameraVertical", No::DeviceType::GamepadAxis, static_cast<int>(No::GamepadAxis::RightStickY), 1.0f, 0.2f);
	No::InputBindAxis("CameraVertical", No::DeviceType::Keyboard, static_cast<int>(VK_UP), 1.0f);
	No::InputBindAxis("CameraVertical", No::DeviceType::Keyboard, static_cast<int>(VK_DOWN), -1.0f);

	No::InputBindAction("CloseLevelUpUI", No::DeviceType::Keyboard, static_cast<int>('X'));
	No::InputBindAction("CloseLevelUpUI", No::DeviceType::GamepadButton, static_cast<int>(No::GamepadButton::X));


	No::InputBindAction("AirDash", No::DeviceType::Keyboard, static_cast<int>(VK_LSHIFT));
	No::InputBindAction("AirDash", No::DeviceType::GamepadButton, static_cast<int>(No::GamepadButton::LB));

	No::InputBindAction("CreateScaffold", No::DeviceType::Keyboard, static_cast<int>('Q'));
	No::InputBindAction("CreateScaffold", No::DeviceType::GamepadButton, static_cast<int>(No::GamepadButton::X));
}
