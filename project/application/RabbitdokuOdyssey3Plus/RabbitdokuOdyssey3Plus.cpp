#include "stdafx.h"
#include "RabbitdokuOdyssey3Plus.h"
#include "Scene/RabbitdokuScene.h"
#include "Scene/RabbitdokuTitleScene.h"
#include "Scene/ExxsearearStage.h"

void RabbitdokuOdyssey3Plus::Startup(void) {
	RegisterScene("Rabbitdoku", []() { return std::make_unique<RabbitdokuScene>();	});
	RegisterScene("ExxsearearStage", []() { return std::make_unique<ExxsearearStage>();	});
	RegisterScene("Title", []() { return std::make_unique<RabbitdokuTitleScene>(); });
	RegisterScene("Stage1", []() { return std::make_unique<ExxsearearStage>(); });
	ChangeScene("Title");
	//ChangeScene("Stage1");

	auto* roomBox = NoEngine::ComponentRegistry::FindByName("RoomTag");
	roomBox->fields[0].attributes.valueSpeed = 1.f;
	roomBox->fields[1].attributes.valueSpeed = 1.f;

	No::InputBindAction("Jump", No::DeviceType::Keyboard, static_cast<int>(VK_LSHIFT));
	No::InputBindAction("Jump", No::DeviceType::Keyboard, static_cast<int>(VK_RSHIFT));
	No::InputBindAction("Jump", No::DeviceType::Keyboard, static_cast<int>(VK_SPACE));
	No::InputBindAction("Jump", No::DeviceType::Gamepad, static_cast<int>(No::GamepadButton::A));

	No::InputBindAction("Right", No::DeviceType::Keyboard, static_cast<int>(VK_RIGHT));
	No::InputBindAction("Right", No::DeviceType::Gamepad, static_cast<int>(No::GamepadButton::Right));
	No::InputBindAction("Left", No::DeviceType::Keyboard, static_cast<int>(VK_LEFT));
	No::InputBindAction("Left", No::DeviceType::Gamepad, static_cast<int>(No::GamepadButton::Left));
	No::InputBindAction("Right", No::DeviceType::Keyboard, static_cast<int>('D'));
	No::InputBindAction("Left", No::DeviceType::Keyboard, static_cast<int>('A'));

	No::InputBindAction("Reset", No::DeviceType::Keyboard, static_cast<int>('R'));
	No::InputBindAction("Reset", No::DeviceType::Gamepad, static_cast<int>(No::GamepadButton::Y));

	No::InputBindAction("Save", No::DeviceType::Keyboard, static_cast<int>(VK_UP));
	No::InputBindAction("EnterDoor", No::DeviceType::Keyboard, static_cast<int>('W'));
	No::InputBindAction("Save", No::DeviceType::Gamepad, static_cast<int>(No::GamepadButton::Up));

	No::InputBindAction("EnterDoor", No::DeviceType::Keyboard, static_cast<int>(VK_UP));
	No::InputBindAction("EnterDoor", No::DeviceType::Keyboard, static_cast<int>('W'));
	No::InputBindAction("EnterDoor", No::DeviceType::Gamepad, static_cast<int>(No::GamepadButton::Up));

	No::SoundLoad(L"resources/game/RabbitdokuOdyssey3Plus/Sounds/SFX/Jump.mp3", "jump");
	No::SoundLoad(L"resources/game/RabbitdokuOdyssey3Plus/Sounds/SFX/JumpDouble.mp3", "doubleJump");
	No::SoundLoad(L"resources/game/RabbitdokuOdyssey3Plus/Sounds/SFX/Death.mp3", "death");
	No::SoundLoad(L"resources/game/RabbitdokuOdyssey3Plus/Sounds/SFX/Save.mp3", "save");
	No::SoundLoad(L"resources/game/RabbitdokuOdyssey3Plus/Sounds/SFX/Spring.mp3", "spring");
	No::SoundLoad(L"resources/game/RabbitdokuOdyssey3Plus/Sounds/SFX/ItemGetJumpReload.mp3", "replenisher");
	No::SoundLoad(L"resources/game/RabbitdokuOdyssey3Plus/Sounds/SFX/BlockBreak.mp3", "collapseBlock");
	No::SoundLoad(L"resources/game/RabbitdokuOdyssey3Plus/Sounds/SFX/Landing.mp3", "landing");

	No::SoundLoad(L"resources/game/RabbitdokuOdyssey3Plus/Sounds/BGM/Title.mp3", "title");

}
