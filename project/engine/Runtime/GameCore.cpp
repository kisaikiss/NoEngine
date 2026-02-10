#include "GameCore.h"
#include "engine/Window/WindowManager.h"
#include "engine/Window/Event/MainEditor/MainEditorWindowCloseEvent.h"
#include "engine/Window/Event/IWindowEvent.h"
#include "engine/Functions/Debug/Logger/Log.h"
#include "engine/Functions/Debug/CrashHandler/ExportDump.h"
#include "engine/Functions/Debug/GraphicsResourceLeakChecker.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Runtime/GpuResource/GpuResource.h"
#include "engine/Runtime/Command/GraphicsContext.h"
#include "engine/Functions/Renderer/RenderPass/RenderPassScheduler.h"
#include "engine/Functions/Input/input.h"
#include "engine/Assets/Audio/Audio.h"

#ifdef USE_IMGUI
#include "engine/Editor/ImGuiManager.h"
#include "externals/imgui/imgui.h"
namespace {
NoEngine::Editor::ImGuiManager imguiManager;
}
#endif // USE_IMGUI

namespace NoEngine {
namespace GameCore {

namespace {
std::chrono::steady_clock::time_point sLastTickTime{ std::chrono::steady_clock::now() };
}


bool IGameApp::Exit() {
	return Input::Keyboard::IsTrigger(VK_ESCAPE);
}

int RunApplication(std::unique_ptr<IGameApp> game) {
	// リソースリークチェッカー
	GraphicsResourceLeakChecker leakCheck;

	EngineInitialize();

	std::unique_ptr<Render::RenderPassScheduler> renderPassScheduler = std::make_unique<Render::RenderPassScheduler>();
	renderPassScheduler->Initialize();

	// ゲームアプリケーションにレンダーパススケジューラを設定します。
	game->SetRenderPassScheduler(renderPassScheduler.get());
	// ゲームアプリケーションの初期化を行います。
	game->Startup();

	RenderContext renderContext;
	CalculateDeltaTime();
	// メインループ
	while (GraphicsCore::gWindowManager.ProcessMessage() == 0) {

		GraphicsContext& context = GraphicsContext::Begin();
		GraphicsCore::gWindowManager.Clear(context);

		InputUpdate();

#ifdef USE_IMGUI
		imguiManager.BeginFrame();
#endif // USE_IMGUI

		const float deltaTime = CalculateDeltaTime();
		game->Update(deltaTime);

		renderContext.SetCamera(game->GetCamera());
		renderPassScheduler->SetRenderContext(renderContext);
		renderPassScheduler->Render(context, game->GetRegistry());

#ifdef USE_IMGUI
		imguiManager.Render(context);
#endif // USE_IMGUI

		
		GraphicsCore::gWindowManager.EndFrame(context);
		if (game->Exit()) break;
	}

	game->Cleanup();
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
	GraphicsCore::gWindowManager.Create(L"NoEngine", 1280, 720, L"resources/engine/noicon.ico");
	GraphicsCore::gWindowManager.SetMainWindowName(L"NoEngine");
#ifdef RELEASE
	// ToDo : チーム制作用にリリースでフルスクリーンを強制しています。ウィンドウモード変更のバグは修正すべきです。
	auto* window = GraphicsCore::gWindowManager.GetMainWindow();
	window->SetWindowMode(Window::WindowMode::kFullScreen);
	window->SetWindowMode(Window::WindowMode::kWindow);
#endif // RELEASE
	
	InputInitialize();
	AudioInitialize();

#ifdef USE_IMGUI
	imguiManager.Initialize();
#endif // USE_IMGUI

}

void EngineFinalize() {
#ifdef USE_IMGUI
	imguiManager.Shutdown();
#endif // USE_IMGUI
	AudioShutdown();
	InputShutdown();
	GraphicsCore::Shutdown();
	CoUninitialize();
}

float CalculateDeltaTime() {
	float deltaTime;
	{
		using namespace std::chrono;

		steady_clock::time_point tickTimePoint = steady_clock::now();
		duration<float> time_span = tickTimePoint - sLastTickTime;
		deltaTime = time_span.count();

		sLastTickTime = tickTimePoint;
	}
	return deltaTime;
}

}
}