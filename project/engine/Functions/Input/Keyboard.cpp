#include "Keyboard.h"

#pragma comment(lib, "gameinput.lib")


namespace NoEngine {
namespace Input {

namespace {
Microsoft::WRL::ComPtr<IGameInput> input;

const size_t kKeyNum = 256;
std::array<bool, kKeyNum> keys;
std::array<bool, kKeyNum> preKeys;
}

void Keyboard::Initialize() {
	HRESULT hr = GameInputCreate(input.GetAddressOf());
	if (FAILED(hr)) {
		assert(false);
	}
}

void Keyboard::Shutdown() {
	input.Reset();
}

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

}
}