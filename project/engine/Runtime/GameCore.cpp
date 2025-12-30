#include "GameCore.h"
#include "engine/Window/WindowManager.h"
#include "engine/Window/Event/MainEditor/MainEditorWindowCloseEvent.h"
#include "engine/Window/Event/IWindowEvent.h"
#include "engine/Debug/Logger/Log.h"
#include "engine/Debug/CrashHandler/ExportDump.h"
#include "engine/Debug/GraphicsResourceLeakChecker.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Runtime/GpuResource/GpuResource.h"
#include "engine/Runtime/Renderer/MeshRenderer.h"
#include "engine/Runtime/Command/GraphicsContext.h"
#ifdef USE_IMGUI
#include "engine/Editor/ImGuiManager.h"
namespace {
NoEngine::Editor::ImGuiManager imguiManager;
}
#endif // USE_IMGUI
namespace NoEngine {
namespace GameCore {



int RunApplication(AllowAccessOnlyFromWinMain) {
	//リソースリークチェッカー
	GraphicsResourceLeakChecker leakCheck;

	EngineInitialize();

	while (GraphicsCore::gWindowManager.ProcessMessage() == 0) {
#ifdef USE_IMGUI
		imguiManager.BeginFrame();
#endif // USE_IMGUI

		
		GraphicsContext& context = GraphicsContext::Begin();
		GraphicsCore::gWindowManager.Clear(context);
		MeshRenderer::Render(context);
#ifdef USE_IMGUI
		imguiManager.Render(context);
#endif // USE_IMGUI

		
		GraphicsCore::gWindowManager.EndFrame(context);
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
	auto* window = GraphicsCore::gWindowManager.Create(L"NoEngine", 1280, 720);
	GraphicsCore::gWindowManager.SetMainWindowName(L"NoEngine");
	window->RegisterWindowEvent(std::make_unique<MainEditorWindowCloseEvent>());
	//sWindowManager->Create(L"NoWindow", 1280, 720);
#ifdef USE_IMGUI
	imguiManager.Initialize();
#endif // USE_IMGUI

	
}

void EngineFinalize() {
#ifdef USE_IMGUI
	imguiManager.Shutdown();
#endif // USE_IMGUI
	GraphicsCore::Shutdown();
	CoUninitialize();
}
}
}