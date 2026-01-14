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
#include "engine/Functions/Camera/Camera.h"
#include "engine/Functions/Input/Keyboard.h"

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

int RunApplication(std::unique_ptr<IGameApp> game) {
	// リソースリークチェッカー
	GraphicsResourceLeakChecker leakCheck;

	EngineInitialize();

	std::unique_ptr<Render::RenderPassScheduler> renderPassScheduler = std::make_unique<Render::RenderPassScheduler>();
	renderPassScheduler->Initialize();

	// ECSのレジストリを生成します。
	std::unique_ptr<ECS::Registry> registry = std::make_unique<ECS::Registry>();
	
	// ゲームアプリケーションの初期化を行います。
	game->SetRegistry(registry.get());
	game->Startup();

	// カメラテスト
	std::unique_ptr<Camera> camera = std::make_unique<Camera>();
	Transform cameraTransform{};
	cameraTransform.rotation = { 0.f,0.f,0.f,1.f };
	cameraTransform.scale = { 1.f,1.f,1.f };
	cameraTransform.translate = { 0.f,0.f,-5.f };
	camera->SetTransform(cameraTransform);

	// メインループ
	while (GraphicsCore::gWindowManager.ProcessMessage() == 0) {

		GraphicsContext& context = GraphicsContext::Begin();
		GraphicsCore::gWindowManager.Clear(context);

		Input::Keyboard::Update();

#ifdef USE_IMGUI
		imguiManager.BeginFrame();

		ImGui::Begin("camera");
		ImGui::DragFloat3("pos", &cameraTransform.translate.x, 0.1f);
		ImGui::End();
		camera->SetTransform(cameraTransform);
#endif // USE_IMGUI

		const float deltaTime = CalculateDeltaTime();
		game->Update(deltaTime);

		camera->Update();

		renderPassScheduler->SetCamera(camera.get());
		renderPassScheduler->Render(context, *registry);

#ifdef USE_IMGUI
		imguiManager.Render(context);
#endif // USE_IMGUI

		
		GraphicsCore::gWindowManager.EndFrame(context);
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
	auto* window = GraphicsCore::gWindowManager.Create(L"NoEngine", 1280, 720);
	GraphicsCore::gWindowManager.SetMainWindowName(L"NoEngine");
	window->RegisterWindowEvent(std::make_unique<MainEditorWindowCloseEvent>());
	//sWindowManager->Create(L"NoWindow", 1280, 720);

	Input::Keyboard::Initialize();

#ifdef USE_IMGUI
	imguiManager.Initialize();
#endif // USE_IMGUI

}

void EngineFinalize() {
#ifdef USE_IMGUI
	imguiManager.Shutdown();
#endif // USE_IMGUI
	Input::Keyboard::Shutdown();
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