#include "Input.h"
#include "engine/Runtime/GraphicsCore.h"

#pragma comment(lib, "gameinput.lib")

namespace NoEngine {

namespace {
Microsoft::WRL::ComPtr<IGameInput> input;

const size_t kKeyNum = 256;
std::array<bool, kKeyNum> keys;
std::array<bool, kKeyNum> preKeys;

GameInputGamepadState gamepadState;
GameInputGamepadState preGamepadState;
Input::Pad::Stick stick;
Input::Pad::Trigger triggerButton;

GameInputMouseState mouseState;
GameInputMouseState preMouseState;


bool isGamepadConnected;
}

void InputInitialize() {
	HRESULT hr = GameInputCreate(input.GetAddressOf());
	if (FAILED(hr)) {
		assert(false);
	}
}

void InputShutdown() {
	input.Reset();
}

void InputUpdate() {
	Input::Keyboard::Update();
	Input::Pad::Update();
	Input::Mouse::Update();
}

namespace Input {
void Keyboard::Update() {
	// 押す状態をリセット
	preKeys = keys;
	keys.fill(false);


	IGameInputReading* reading = nullptr;
	if (SUCCEEDED(input->GetCurrentReading(GameInputKindKeyboard, nullptr, &reading))) {
		// 押すキーの数
		uint32_t keyCount = reading->GetKeyCount();
		if (keyCount > 0) {
			GameInputKeyState keyState[16];

			// 押すキー状態
			reading->GetKeyState(_countof(keyState), keyState);
			for (uint32_t i = 0; i < keyCount; i++) {
				uint8_t virtualKey = keyState[i].virtualKey;
				keys[virtualKey] = true;
			}
		}
		reading->Release();
	}
}

bool Keyboard::IsTrigger(uint8_t keyCode) {
	return keys[keyCode] && !preKeys[keyCode];
}

bool Keyboard::IsPress(uint8_t keyCode) {
	return keys[keyCode];
}

void Pad::Update() {
	preGamepadState = gamepadState;

	IGameInputReading* readingPad = nullptr;
	if (SUCCEEDED(input->GetCurrentReading(GameInputKindGamepad, nullptr, &readingPad))) {
		readingPad->GetGamepadState(&gamepadState);
		readingPad->Release();
		isGamepadConnected = true;

		triggerButton.leftTrigger = gamepadState.leftTrigger;
		triggerButton.rightTrigger = gamepadState.rightTrigger;
		stick.leftStickX = gamepadState.leftThumbstickX;
		stick.leftStickY = gamepadState.leftThumbstickY;
		stick.rightStickX = gamepadState.rightThumbstickX;
		stick.rightStickY = gamepadState.rightThumbstickY;

	} else {
		gamepadState = {};
		stick = {};
		triggerButton = {};
		isGamepadConnected = false;
	}


}

bool Pad::IsTrigger(GamepadButton button) {
	switch (button) {
	case GamepadButton::Left:	return (gamepadState.buttons & GameInputGamepadDPadLeft) && !(preGamepadState.buttons & GameInputGamepadDPadLeft);				break;
	case GamepadButton::Right:	return (gamepadState.buttons & GameInputGamepadDPadRight) && !(preGamepadState.buttons & GameInputGamepadDPadRight);			break;
	case GamepadButton::Up:		return (gamepadState.buttons & GameInputGamepadDPadUp) && !(preGamepadState.buttons & GameInputGamepadDPadUp);					break;
	case GamepadButton::Down:	return (gamepadState.buttons & GameInputGamepadDPadDown) && !(preGamepadState.buttons & GameInputGamepadDPadDown);				break;
	case GamepadButton::A:		return (gamepadState.buttons & GameInputGamepadA) && !(preGamepadState.buttons & GameInputGamepadA);							break;
	case GamepadButton::B:		return (gamepadState.buttons & GameInputGamepadB) && !(preGamepadState.buttons & GameInputGamepadB);							break;
	case GamepadButton::X:		return (gamepadState.buttons & GameInputGamepadX) && !(preGamepadState.buttons & GameInputGamepadX);							break;
	case GamepadButton::Y:		return (gamepadState.buttons & GameInputGamepadY) && !(preGamepadState.buttons & GameInputGamepadY);							break;
	case GamepadButton::RB:		return (gamepadState.buttons & GameInputGamepadRightShoulder) && !(preGamepadState.buttons & GameInputGamepadRightShoulder);	break;
	case GamepadButton::LB:		return (gamepadState.buttons & GameInputGamepadLeftShoulder) && !(preGamepadState.buttons & GameInputGamepadLeftShoulder);		break;
	case GamepadButton::Start:	return (gamepadState.buttons & GameInputGamepadMenu) && !(preGamepadState.buttons & GameInputGamepadMenu);						break;
	case GamepadButton::Select:	return (gamepadState.buttons & GameInputGamepadView) && !(preGamepadState.buttons & GameInputGamepadView);						break;
	}
	return false;
}

bool Pad::IsPress(GamepadButton button) {
	switch (button) {
	case GamepadButton::Left:	return gamepadState.buttons & GameInputGamepadDPadLeft;		break;
	case GamepadButton::Right:	return gamepadState.buttons & GameInputGamepadDPadRight;	break;
	case GamepadButton::Up:		return gamepadState.buttons & GameInputGamepadDPadUp;		break;
	case GamepadButton::Down:	return gamepadState.buttons & GameInputGamepadDPadDown;		break;
	case GamepadButton::A:		return gamepadState.buttons & GameInputGamepadA;			break;
	case GamepadButton::B:		return gamepadState.buttons & GameInputGamepadB;			break;
	case GamepadButton::X:		return gamepadState.buttons & GameInputGamepadX;			break;
	case GamepadButton::Y:		return gamepadState.buttons & GameInputGamepadY;			break;
	case GamepadButton::RB:		return gamepadState.buttons & GameInputGamepadRightShoulder; break;
	case GamepadButton::LB:		return gamepadState.buttons & GameInputGamepadLeftShoulder;	break;
	case GamepadButton::Start:	return gamepadState.buttons & GameInputGamepadMenu;			break;
	case GamepadButton::Select:	return gamepadState.buttons & GameInputGamepadView;			break;
	}
	return false;
}

const Pad::Trigger& Pad::GetTriggerButton() {
	return triggerButton;
}

const Pad::Stick& Pad::GetStick() {
	return stick;
}

bool Pad::IsGamepadConnected() {
	return isGamepadConnected;
}

bool Mouse::IsTrigger(MouseButton button) {
	switch (button) {
	case NoEngine::Input::MouseButton::Left:	return (mouseState.buttons & GameInputMouseLeftButton) && !(preMouseState.buttons & GameInputMouseLeftButton);		break;
	case NoEngine::Input::MouseButton::Right:	return (mouseState.buttons & GameInputMouseRightButton) && !(preMouseState.buttons & GameInputMouseRightButton);	break;
	case NoEngine::Input::MouseButton::Middle:	return (mouseState.buttons & GameInputMouseMiddleButton) && !(preMouseState.buttons & GameInputMouseMiddleButton);	break;
	}
	return false;
}

bool Mouse::IsPress(MouseButton button) {
	switch (button) {
	case NoEngine::Input::MouseButton::Left:	return mouseState.buttons & GameInputMouseLeftButton;	break;
	case NoEngine::Input::MouseButton::Right:	return mouseState.buttons & GameInputMouseRightButton;	break;
	case NoEngine::Input::MouseButton::Middle:	return mouseState.buttons & GameInputMouseMiddleButton;	break;
	}
	return false;
}

bool Mouse::GetPosition(int* mouseX, int* mouseY) {
	if (mouseX && mouseY) {
		// スクリーンを座標系からウィンドウ座標に変換
		POINT cursorPos;
		GetCursorPos(&cursorPos); // スクリーンを得る
		ScreenToClient(GraphicsCore::gWindowManager.GetMainWindow()->GetWindowHandle(), &cursorPos); // ウィンドウ座標に変換

		*mouseX = static_cast<int>(cursorPos.x);
		*mouseY = static_cast<int>(cursorPos.y);
		return true;
	}

	return false;
}

float Mouse::GetMouseWheelX() {
	return static_cast<float>(mouseState.wheelX);
}

float Mouse::GetMouseWheelY() {
	return static_cast<float>(mouseState.wheelY);
}

void Mouse::Update() {
	preMouseState = mouseState;

	IGameInputReading* readingMouse = nullptr;
	if (SUCCEEDED(input->GetCurrentReading(GameInputKindMouse, nullptr, &readingMouse))) {
		readingMouse->GetMouseState(&mouseState);
		readingMouse->Release();
	}
}

}
}