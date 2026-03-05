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

enum class MouseButton {
	Left = 0,
	Right,
	Middle,
	NONE
};

class Mouse {
	friend void NoEngine::InputUpdate();
public:
	static bool IsTrigger(MouseButton button);

	static bool IsPress(MouseButton button);

	/// <summary>
	/// マウスのスクリーン上の座標を取得
	/// </summary>
	/// <param name="mouseX">X座標の戻り値引数</param>
	/// <param name="mouseY">Y座標の戻り値引数</param>
	/// <returns>成功したか(true : 成功, false : 失敗)</returns>
	static bool GetPosition(int* mouseX, int* mouseY);

	static float GetMouseWheelX();
	static float GetMouseWheelY();

private:
	static void Update();
};

}
}