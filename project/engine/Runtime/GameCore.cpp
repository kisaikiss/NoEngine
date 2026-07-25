#include "GameCore.h"
#include "engine/Window/WindowManager.h"
#include "engine/Functions/Debug/CrashHandler/ExportDump.h"
#include "engine/Functions/Debug/GraphicsResourceLeakChecker.h"
#include "engine/Functions/Particle/ParticleManager.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Runtime/Command/GraphicsContext.h"
#include "engine/Functions/Renderer/RenderPass/RenderPassScheduler.h"
#include "engine/Functions/Renderer/RenderSystem.h"
#include "engine/Functions/Input/input.h"
#include "engine/Editor/EditorCommandOperator.h"
#include "engine/Assets/Audio/Audio.h"
#include "engine/Assets/AssetBrowserWindow.h"
#include "engine/Assets/AssetManager.h"

#ifdef USE_IMGUI
#include "engine/Editor/ImGuiManager.h"
#include "externals/imgui/imgui.h"
namespace {
NoEngine::ImGuiManager sImGuiManager;
}
#endif // USE_IMGUI

namespace NoEngine {
namespace GameCore {

namespace {
std::chrono::steady_clock::time_point sLastTickTime{ std::chrono::steady_clock::now() };

static float sElapsedTime = 0.f;
static float sFrameTimes[120] = {};
static int sFrameIndex = 0;
}


void IGameApp::SetupRenderPass(RenderPassScheduler& renderPassScheduler) {
	CommonSetupRenderPass(renderPassScheduler);
	CommonSetupDebugRenderPass(renderPassScheduler);
}

bool IGameApp::Exit() {
	return Input::Keyboard::IsTrigger(VK_ESCAPE);
}

int RunApplication(std::unique_ptr<IGameApp> game) {
	// リソースリークチェッカー
	GraphicsResourceLeakChecker leakCheck;

	EngineInitialize();

	RenderPassScheduler renderPassScheduler;

	// ゲームアプリケーションの初期化を行います。
	game->Startup();
	game->SetupRenderPass(renderPassScheduler);
	renderPassScheduler.Compile();
	
	CalculateDeltaTime();
	// メインループ
	while (GraphicsCore::sWindowManager.ProcessMessage() == 0) {

		GraphicsCore::CheckDeviceStatus();

		UINT newWidth, newHeight;
		if (GraphicsCore::sWindowManager.GetMainWindow()->ConsumeResizeRequest(newWidth, newHeight)) {
			GraphicsCore::Resize(newWidth, newHeight);
			renderPassScheduler.Resize(newWidth, newHeight);
			Render::ResizeRaytracingDesc(newWidth, newHeight);
		}

		GraphicsContext& context = GraphicsContext::Begin();
		GraphicsCore::StartFrame(context);
		ComputeContext& ctx = ComputeContext::Begin(L"MainComputeContext", true);

		InputUpdate();

#ifdef USE_IMGUI
		sImGuiManager.BeginFrame();
		Editor::DrawAssetBrowserWindow(game->GetRegistry());
#endif // USE_IMGUI

		float deltaTime = CalculateDeltaTime();
		sElapsedTime += deltaTime;
		game->Update(ctx, deltaTime);
		
		ctx.Finish(true);

		Editor::EditorCommandOperator::Update(deltaTime);
		renderPassScheduler.Render(context, game->GetRegistry());

		DrawPerformance(deltaTime);
		Log::GetInstance().DrawImGuiWindow("Console");

		GraphicsCore::EndFrame(context, renderPassScheduler.GetScreenDrawBuffer());
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
	Log::GetInstance().Initialize();
	Log::GetInstance().SetVerbosityLevel(VerbosityLevel::kDebug);

	// グラフィックス関連の基盤を初期化
	GraphicsCore::Initialize();

	
	InputInitialize();
	AudioInitialize();
	
	// 初期化に使うComputeContext
	ComputeContext& ctx = ComputeContext::Begin(L"InitializeComputeContext");
	ParticleManager::Initialize(ctx);
	ctx.Finish();

	AssetManager::CreateAddressableList();
	
#ifdef USE_IMGUI
	Editor::RefreshAssetBrowser();
	sImGuiManager.Initialize();
#endif // USE_IMGUI
}

void EngineFinalize() {
#ifdef USE_IMGUI
	sImGuiManager.Shutdown();
#endif // USE_IMGUI
	Editor::EditorCommandOperator::Shutdown();
	ParticleManager::Shutdown();
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

void DrawPerformance(float deltaTime) {
#ifdef USE_IMGUI

	sFrameTimes[sFrameIndex] = deltaTime * 1000.0f; // msに変換
	sFrameIndex = (sFrameIndex + 1) % IM_ARRAYSIZE(sFrameTimes);

	ImGui::Begin("Performance");
	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	ImGui::PlotLines(
		"Frame Time (ms)",
		sFrameTimes,
		IM_ARRAYSIZE(sFrameTimes),
		sFrameIndex,
		nullptr,
		0.0f,
		40.0f,   // Y軸の最大値（40ms = 25FPS）
		ImVec2(0, 80)
	);
	ImGui::End();
#else
	static_cast<void>(deltaTime);
#endif // USE_IMGUI

}

float GetElapsedTime() {
	return sElapsedTime;
}

}
}