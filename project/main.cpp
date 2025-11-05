#include "engine/Window/WindowManager.h"
#include "engine/Window/Event/MainEditor/MainEditorWindowCloseEvent.h"
#include "engine/Window/Event/IWindowEvent.h"
#include "engine/Debug/Logger/Log.h"
#include "engine/Debug/CrashHandler/ExportDump.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	// 誰も捕捉しなかった場合に(Unhandled)、捕捉する関数を登録
	// main関数が始まってすぐに登録する
	SetUnhandledExceptionFilter(NoEngine::ExportDump);
	NoEngine::Log::Initialize();
	NoEngine::Log::SetVerbosityLevel(NoEngine::VerbosityLevel::kDebug);
	std::unique_ptr<NoEngine::WindowManager> windowManager;
	windowManager = std::make_unique<NoEngine::WindowManager>();
	auto* window = windowManager->Create(L"NoEngine", 1280, 720);
	windowManager->SetMainWindowName(L"NoEngine");
	window->RegisterWindowEvent(std::make_unique<NoEngine::MainEditorWindowCloseEvent>());

	while (windowManager->ProcessMessage() == 0) {

	}
	return 0;
}