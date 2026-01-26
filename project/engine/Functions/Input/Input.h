#pragma once
#include <externals/GameInput/GameInput.h>


namespace NoEngine {
void InputInitialize();

void InputShutdown();

void InputUpdate();
namespace Input {

class Keyboard {
	friend void NoEngine::InputUpdate();
public:

	static bool IsTrigger(uint8_t keyCode);

	static bool IsPress(uint8_t keyCode);
private:
	static void Update();
};


enum class GamepadButton {
	Left = 0,
	Right,
	Up,
	Down,
	A,
	B,
	X,
	Y,
	RB,
	LB,
	Start,
	Select,
};

class Pad {
	friend void NoEngine::InputUpdate();
public:
	struct Stick {	
		float leftStickX;
		float leftStickY;
		float rightStickX;
		float rightStickY;
	};

	struct Trigger {
		float leftTrigger;
		float rightTrigger;
	};

	static bool IsTrigger(GamepadButton button);

	static bool IsPress(GamepadButton button);

	static const Trigger& GetTriggerButton();

	static const Stick& GetStick();

	static bool IsGamepadConnected();
private:
	static void Update();
};

}
}