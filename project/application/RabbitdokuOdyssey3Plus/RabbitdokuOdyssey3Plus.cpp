#include "stdafx.h"
#include "RabbitdokuOdyssey3Plus.h"
#include "Scene/RabbitdokuScene.h"
#include "Scene/RabbitdokuTitleScene.h"
#include "Scene/ExxsearearStage.h"
#include "Game/RabbitdokuTag.h"

class Stage1 : public RabbitdokuScene {
public: Stage1() : RabbitdokuScene("stage1") {}
};

class Stage2 : public RabbitdokuScene {
public: Stage2() : RabbitdokuScene("stage2") {}
};

class Stage3 : public RabbitdokuScene {
public: Stage3() : RabbitdokuScene("stage3") {}
};

class Stage4 : public RabbitdokuScene {
public: Stage4() : RabbitdokuScene("stage4") {}
};

class SudokuStage : public RabbitdokuScene {
public: SudokuStage() : RabbitdokuScene("sudokuStage",0.25f) {}
};

class ExStage : public RabbitdokuScene {
public: ExStage() : RabbitdokuScene("exStage", 0.25f) {}
private: void InitBackground(No::Registry& registry) override {
	auto e = registry.GenerateEntity();
	auto* t = registry.AddComponent<No::Transform2DComponent>(e);
	t->scale = No::Vector2(1280.f, 768.f);
	auto* s = registry.AddComponent<No::SpriteComponent>(e);
	s->textureFilePath = "resources/game/RabbitdokuOdyssey3Plus/Sprite/exBackground.png";
	s->layer = 0;
	registry.AddComponent<MainBackgroundTag>(e);
	registry.AddComponent<ExBackgroundTag>(e);
}
};

void RabbitdokuOdyssey3Plus::Startup(void) {
	No::SoundLoad(L"resources/game/RabbitdokuOdyssey3Plus/Sounds/BGM/Title.mp3", "title");
	No::SoundLoad(L"resources/game/RabbitdokuOdyssey3Plus/Sounds/BGM/Stage01.mp3", "stage1");
	No::SoundLoad(L"resources/game/RabbitdokuOdyssey3Plus/Sounds/BGM/Stage02.mp3", "stage2");
	No::SoundLoad(L"resources/game/RabbitdokuOdyssey3Plus/Sounds/BGM/Stage01.mp3", "stage3");
	No::SoundLoad(L"resources/game/RabbitdokuOdyssey3Plus/Sounds/BGM/Stage01.mp3", "stage4");
	No::SoundLoad(L"resources/game/RabbitdokuOdyssey3Plus/Sounds/BGM/ExStage.mp3", "exStage");
	No::SoundLoad(L"resources/game/RabbitdokuOdyssey3Plus/Sounds/BGM/SudokuStage.mp3", "sudokuStage");

	RegisterScene("Rabbitdoku", []() { return std::make_unique<RabbitdokuScene>();	});
	RegisterScene("ExxsearearStage", []() { return std::make_unique<ExxsearearStage>();	});
	RegisterScene("Title", []() { return std::make_unique<RabbitdokuTitleScene>(); });
	RegisterScene("Stage1", []() { return std::make_unique<Stage1>(); });
	RegisterScene("Stage2", []() { return std::make_unique<Stage2>(); });
	RegisterScene("Stage3", []() { return std::make_unique<Stage3>(); });
	RegisterScene("Stage4", []() { return std::make_unique<Stage4>(); });
	RegisterScene("SudokuStage", []() { return std::make_unique<SudokuStage>(); });
	RegisterScene("ExStage", []() { return std::make_unique<ExStage>(); });

	ChangeScene("Stage3");

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
	No::SoundLoad(L"resources/game/RabbitdokuOdyssey3Plus/Sounds/SFX/ClearItemGet.mp3", "goalGet");
	No::SoundLoad(L"resources/game/RabbitdokuOdyssey3Plus/Sounds/SFX/ClearJingle.mp3", "jingle");



}
