#include "GraphicsCore.h"

#include "Command/CommandListManager.h"
#include "ContextManager.h"
#include "engine/Functions/Renderer/RenderSystem.h"
#include "engine/Runtime/GpuResource/LinearAllocator/LinearAllocator.h"
#include "Graphics/GraphicsCommon.h"

#include "engine/Functions/Debug/Logger/Log.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI



namespace NoEngine {
using namespace std;

std::unique_ptr<Graphics::GraphicsInfrastructures> GraphicsCore::sGraphicsInfrastructures;
std::unique_ptr<Graphics::GraphicsDevice> GraphicsCore::sGraphicsDevice;
CommandListManager GraphicsCore::sCommandListManager;
ContextManager GraphicsCore::sContextManager;
WindowManager GraphicsCore::sWindowManager;

namespace {
const uint32_t sSwapChainBufferCount = 2;

std::unique_ptr<Graphics::GraphicsSwapChain> sSwapChain;
std::array<std::unique_ptr<ColorBuffer>, sSwapChainBufferCount> sColorBuffers;
std::unique_ptr<DepthBuffer> sDepthBuffer;

// ビューポート
D3D12_VIEWPORT sViewport;
// シザー矩形
D3D12_RECT sScissorRect;

UINT sBackBufferIndex;
}

DescriptorAllocator GraphicsCore::sDescriptorAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
{
	D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
	D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
	D3D12_DESCRIPTOR_HEAP_TYPE_DSV
};

void GraphicsCore::Initialize() {
	EnableDebugLayer();
	sGraphicsInfrastructures = make_unique<Graphics::GraphicsInfrastructures>();
	sGraphicsDevice = make_unique<Graphics::GraphicsDevice>(sGraphicsInfrastructures->GetDXGIAdapter());
	sCommandListManager.Create();
	SettingDebugLayer();
	Render::Initialize();
	GraphicsCore::sWindowManager.Create(L"NoEngine", 1280, 720, L"resources/engine/noicon.ico");
	GraphicsCore::sWindowManager.SetMainWindowName(L"NoEngine");

	uint32_t windowWidth = 1280;
	uint32_t windowHeight = 720;
	sSwapChain = make_unique<Graphics::GraphicsSwapChain>(sWindowManager.GetMainWindow()->GetWindowHandle(),1280.f, 720.f, sSwapChainBufferCount);

	// viewportをウィンドウサイズと同じにします。
	sViewport.Width = static_cast<FLOAT>(windowWidth);
	sViewport.Height = static_cast<FLOAT>(windowHeight);
	sViewport.TopLeftX = 0.f;
	sViewport.TopLeftY = 0.f;
	sViewport.MinDepth = 0.f;
	sViewport.MaxDepth = 1.f;

	// シザー矩形はビューポートと同じ大きさにします。
	sScissorRect.left = 0;
	sScissorRect.right = windowWidth;
	sScissorRect.top = 0;
	sScissorRect.bottom = windowHeight;

	CreatePixelBuffer();
}

void GraphicsCore::Shutdown(void) {
	DestroyPixelBuffer();
	sSwapChain.reset();
	sWindowManager.Shutdown();
	Render::Shutdown();
	CommandContext::DestroyAllContexts();

	for (auto& descriptorAllocator : sDescriptorAllocator) {
		descriptorAllocator.DestroyAll();
	}


	sCommandListManager.Shutdown();

	sGraphicsDevice.reset();
	sGraphicsInfrastructures.reset();
}

void GraphicsCore::EnableDebugLayer() {
#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		// デバッグレイヤーを有効化する
		debugController->EnableDebugLayer();
		// さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif
}

void GraphicsCore::SettingDebugLayer() {
#ifdef _DEBUG
	if (!sGraphicsDevice) {
		Log::DebugPrint("GraphicsDevice is nulptr!!!", VerbosityLevel::kCritical);
		assert(false);
	}
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(sGraphicsDevice->GetDevice()->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		// ヤバイエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		// エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		// 警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		// 抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			// Windows11でのDXGIデバッガーレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
			// https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-windows-11
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};

		// 抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;

		// 指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);

		// 解放
		infoQueue->Release();
	}
#endif
}

void GraphicsCore::StartFrame(GraphicsContext& context) {
	sBackBufferIndex = sSwapChain->GetSwapChain()->GetCurrentBackBufferIndex();
	context.TransitionResource(*sColorBuffers[sBackBufferIndex].get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	context.TransitionResource(*sDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	context.SetRenderTarget(sColorBuffers[sBackBufferIndex]->GetRTV(), sDepthBuffer->GetDSV());

	context.SetViewportAndScissor(sViewport, sScissorRect);
	context.ClearColor(*sColorBuffers[sBackBufferIndex].get());
	context.ClearDepthAndStencil(*sDepthBuffer);
}

void GraphicsCore::EndFrame(GraphicsContext& context) {
	context.TransitionResource(*sColorBuffers[sBackBufferIndex].get(), D3D12_RESOURCE_STATE_PRESENT);
	context.Finish(true);
#ifdef USE_IMGUI
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
#endif
	sSwapChain->Get()->Present(1, 0);
}

void GraphicsCore::CreatePixelBuffer() {
	if (sColorBuffers[0]) return;

	// ウィンドウ専用のカラーバッファを生成します。
	for (uint32_t i = 0; i < sSwapChainBufferCount; i++) {
		Microsoft::WRL::ComPtr<ID3D12Resource> displayPlane;
		HRESULT hr = sSwapChain->Get()->GetBuffer(i, IID_PPV_ARGS(&displayPlane));
		if (FAILED(hr)) {
			Log::DebugPrint("swap chain GetBuffer() failed", VerbosityLevel::kCritical);
			assert(false);
		}
		sColorBuffers[i] = std::make_unique<ColorBuffer>();
		sColorBuffers[i]->CreateFromSwapChain(L"Primary SwapChain Buffer", displayPlane.Detach());
	}
	sDepthBuffer = std::make_unique<DepthBuffer>(1.f);
	// ToDo : ウィンドウサイズがマジックナンバーになっている
	sDepthBuffer->Create(L"Window Depth Buffer", 1280, 720, DXGI_FORMAT_D24_UNORM_S8_UINT);

	Log::DebugPrint("create pixel buffers");
}

void GraphicsCore::DestroyPixelBuffer() {
	GraphicsCore::sCommandListManager.IdleGPU();
	for (auto& colorBuffer : sColorBuffers) {
		colorBuffer.reset();
	}
	sDepthBuffer.reset();
	Log::DebugPrint("destroy pixel buffers");
}

}