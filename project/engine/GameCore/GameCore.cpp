#include "GameCore.h"
#include "engine/Window/WindowManager.h"
#include "engine/Window/Event/MainEditor/MainEditorWindowCloseEvent.h"
#include "engine/Window/Event/IWindowEvent.h"
#include "engine/Debug/Logger/Log.h"
#include "engine/Debug/CrashHandler/ExportDump.h"
#include "engine/GameCore/GraphicsCore.h"
#include "engine/GameCore/GpuResource/GpuResource.h"

namespace NoEngine {
int GameCore::RunApplication(AllowAccessOnlyFromWinMain) {
	// 誰も捕捉しなかった場合に(Unhandled)、捕捉する関数を登録します。
	SetUnhandledExceptionFilter(ExportDump);

	// ログを初期化します。
	Log::Initialize();
	Log::SetVerbosityLevel(VerbosityLevel::kDebug);

	// ウィンドウの生成、初期化を行います。
	std::unique_ptr<WindowManager> windowManager;
	windowManager = std::make_unique<WindowManager>();
	auto* window = windowManager->Create(L"NoEngine", 1280, 720);
	windowManager->SetMainWindowName(L"NoEngine");
	window->RegisterWindowEvent(std::make_unique<MainEditorWindowCloseEvent>());

	// グラフィックス関連の基盤を初期化
	GraphicsCore::Initialize();

	while (windowManager->ProcessMessage() == 0) {

	}
    return 0;
}
}