#include "GameCore.h"
#include "engine/Window/WindowManager.h"
#include "engine/Window/Event/MainEditor/MainEditorWindowCloseEvent.h"
#include "engine/Window/Event/IWindowEvent.h"
#include "engine/Debug/Logger/Log.h"
#include "engine/Debug/CrashHandler/ExportDump.h"
#include "engine/Debug/GraphicsResourceLeakChecker.h"
#include "engine/GameCore/GraphicsCore.h"
#include "engine/GameCore/GpuResource/GpuResource.h"

namespace NoEngine {
namespace GameCore {

namespace {
std::unique_ptr<WindowManager> sWindowManager;
}

int RunApplication(AllowAccessOnlyFromWinMain) {
	//リソースリークチェッカー
	GraphicsResourceLeakChecker leakCheck;

	EngineInitialize();

	while (sWindowManager->ProcessMessage() == 0) {

	}

	EngineFinalize();

	return 0;
}

void EngineInitialize() {
	// 誰も捕捉しなかった場合に(Unhandled)、捕捉する関数を登録します。
	SetUnhandledExceptionFilter(ExportDump);

	//COM初期化
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
		assert(false);
	}

	// ログを初期化します。
	Log::Initialize();
	Log::SetVerbosityLevel(VerbosityLevel::kDebug);

	// グラフィックス関連の基盤を初期化
	GraphicsCore::Initialize();

	// ウィンドウの生成、初期化を行います。
	sWindowManager = std::make_unique<WindowManager>();
	auto* window = sWindowManager->Create(L"NoEngine", 1280, 720);
	sWindowManager->SetMainWindowName(L"NoEngine");
	window->RegisterWindowEvent(std::make_unique<MainEditorWindowCloseEvent>());

	
}

void EngineFinalize() {
	
	GraphicsCore::Shutdown();
	sWindowManager->Shutdown();
	sWindowManager.reset();
}
}
}