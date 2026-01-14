#pragma once
#include <externals/GameInput/GameInput.h>


namespace NoEngine {
namespace Input {
class Keyboard {
public:
	static void Initialize();

	static void Shutdown();

	static void Update();

	static bool IsTrigger(uint8_t keyCode);

	static bool IsPress(uint8_t keyCode);
};
}
}