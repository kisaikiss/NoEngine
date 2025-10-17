#include "engine/Windows/WindowManager.h"

#include <memory>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	std::unique_ptr<NoEngine::WindowManager> windowManager;
	windowManager->Initialize(L"NoEngine");

	while (windowManager->ProcessMessage() == 0) {

	}
	return 0;
}