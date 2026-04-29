#include "GraphicsCore.h"

#include "Command/CommandListManager.h"
#include "ContextManager.h"
#include "engine/Functions/Renderer/RenderSystem.h"
#include "engine/Functions/Shader/ShaderModule.h"

#include "engine/Functions/Debug/Logger/Log.h"
#include "engine/Utilities/Conversion/ConvertString.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
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
std::array<std::unique_ptr<ColorBuffer>, sSwapChainBufferCount> sFrameBuffers; // 実際に描画するカラーバッファ
std::unique_ptr<DepthBuffer> sDepthBuffer;									   // 深度バッファ
ColorBuffer sShadowMaskBuffer;
ColorBuffer sPostEffectBuffer;												   // ポストエフェクト用カラーバッファ

// ビューポート
D3D12_VIEWPORT sViewport;
// シザー矩形
D3D12_RECT sScissorRect;

UINT sBackBufferIndex;

// WindowSize
float sWindowWidth;
float sWindowHeight;

GraphicsPSO defaultPSO(L"CopyImage");
std::unique_ptr<RootSignature> defaultRootSignature;
#ifdef USE_IMGUI
ImTextureID sPostEffectTexture;
#endif // USE_IMGUI

// レイトレーシングが有効か
bool sIsEnableRaytracing = false;

}

DescriptorAllocator GraphicsCore::sDescriptorAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
{
	D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
	D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
	D3D12_DESCRIPTOR_HEAP_TYPE_DSV
};

void GraphicsCore::Initialize(float windowWidth, float windowHeight) {
	EnableDebugLayer();
	sGraphicsInfrastructures = make_unique<Graphics::GraphicsInfrastructures>();
	sGraphicsDevice = make_unique<Graphics::GraphicsDevice>(sGraphicsInfrastructures->GetDXGIAdapter());
	sCommandListManager.Create();
	SettingDebugLayer();
	CheckRaytracingEnable();
	Render::Initialize();
	GraphicsCore::sWindowManager.Create(L"NoEngine", UINT(windowWidth), UINT(windowHeight), L"resources/engine/noicon.ico");
	GraphicsCore::sWindowManager.SetMainWindowName(L"NoEngine");

	sSwapChain = make_unique<Graphics::GraphicsSwapChain>(sWindowManager.GetMainWindow()->GetWindowHandle(), windowWidth, windowHeight, sSwapChainBufferCount);

	// viewportをウィンドウサイズと同じにします。
	sViewport.Width = static_cast<FLOAT>(windowWidth);
	sViewport.Height = static_cast<FLOAT>(windowHeight);
	sViewport.TopLeftX = 0.f;
	sViewport.TopLeftY = 0.f;
	sViewport.MinDepth = 0.f;
	sViewport.MaxDepth = 1.f;

	// シザー矩形はビューポートと同じ大きさにします。
	sScissorRect.left = 0;
	sScissorRect.right = static_cast<LONG>(windowWidth);
	sScissorRect.top = 0;
	sScissorRect.bottom = static_cast<LONG>(windowHeight);

	sWindowWidth = windowWidth;
	sWindowHeight = windowHeight;

	sPostEffectBuffer = ColorBuffer();
	sPostEffectBuffer.Create(L"PostEffect Buffer", static_cast<uint32_t>(windowWidth), static_cast<uint32_t>(windowHeight), 1, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	sPostEffectBuffer.CreateImGuiSRV();

	CreatePixelBuffer();

	InitPostEffect();
}

void GraphicsCore::Shutdown(void) {
	DestroyPixelBuffer();
	sSwapChain.reset();
	sPostEffectBuffer.Destroy();
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
	// オブジェクトの描画開始
	context.TransitionResource(sPostEffectBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	context.TransitionResource(*sDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	context.SetRenderTarget(sPostEffectBuffer.GetRTV(), sDepthBuffer->GetDSV());

	context.SetViewportAndScissor(sViewport, sScissorRect);
	context.ClearColor(sPostEffectBuffer);
	context.ClearDepthAndStencil(*sDepthBuffer);
}

void GraphicsCore::EndFrame(GraphicsContext& context) {
	context.TransitionResource(sPostEffectBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	// 本描画
	sBackBufferIndex = sSwapChain->GetSwapChain()->GetCurrentBackBufferIndex();
	context.TransitionResource(*sFrameBuffers[sBackBufferIndex].get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	context.SetRenderTarget(sFrameBuffers[sBackBufferIndex]->GetRTV());

	context.SetViewportAndScissor(sViewport, sScissorRect);
	context.ClearColor(*sFrameBuffers[sBackBufferIndex].get());

	FullScreenDraw(context);

#ifdef USE_IMGUI
	static bool isInitFrame = true;
	if (isInitFrame) {
		// gTextureHeap.Alloc() で空きスロットを確保。
		NoEngine::DescriptorHandle slot = Render::gTextureHeap.Alloc();
		sGraphicsDevice->GetDevice()->CopyDescriptorsSimple(
			1,
			static_cast<D3D12_CPU_DESCRIPTOR_HANDLE>(slot),
			sPostEffectBuffer.GetImGuiSRV(),
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		sPostEffectTexture = static_cast<ImTextureID>(slot.GetGpuPtr());
		isInitFrame = false;
	}



	ImGui::Begin("Game");
	ImGui::Image(
		sPostEffectTexture,
		ImVec2(sWindowWidth * 2 / 3, sWindowHeight * 2 / 3) // 表示サイズ
	);
	ImGui::End();
	ImGui::Render();
	context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, Render::gTextureHeap.GetHeapPointer());
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), context.GetCommandList());
#endif

	context.TransitionResource(*sFrameBuffers[sBackBufferIndex].get(), D3D12_RESOURCE_STATE_PRESENT);
	context.Finish(true);

#ifdef USE_IMGUI
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
#endif
	sSwapChain->Get()->Present(1, 0);
}

bool GraphicsCore::IsEnableRaytracing() {
	return sIsEnableRaytracing;
}

void GraphicsCore::CheckDeviceStatus() {
	HRESULT hr = sGraphicsDevice->GetDevice()->GetDeviceRemovedReason();

	if (SUCCEEDED(hr)) {
		return; //!< 正常
	}

	Log::DebugPrint(GetHResultMessage(hr), VerbosityLevel::kCritical);
	assert(false);
}

void GraphicsCore::CreatePixelBuffer() {
	if (sFrameBuffers[0]) return;

	// スワップチェーンからバッファを生成します。
	for (uint32_t i = 0; i < sSwapChainBufferCount; i++) {
		Microsoft::WRL::ComPtr<ID3D12Resource> displayPlane;
		HRESULT hr = sSwapChain->Get()->GetBuffer(i, IID_PPV_ARGS(&displayPlane));
		if (FAILED(hr)) {
			Log::DebugPrint("swap chain GetBuffer() failed", VerbosityLevel::kCritical);
			assert(false);
		}
		sFrameBuffers[i] = std::make_unique<ColorBuffer>();
		sFrameBuffers[i]->CreateFromSwapChain(L"Primary SwapChain Buffer", displayPlane.Detach());
	}
	sDepthBuffer = std::make_unique<DepthBuffer>(1.f);
	
	sDepthBuffer->Create(L"GraphicsCore Depth Buffer", static_cast<uint32_t>(sWindowWidth), static_cast<uint32_t>(sWindowHeight), DXGI_FORMAT_D24_UNORM_S8_UINT);
	sShadowMaskBuffer.Create(L"ShadowMask", static_cast<uint32_t>(sWindowWidth), static_cast<uint32_t>(sWindowHeight), 1, DXGI_FORMAT_R8_UNORM);
	Log::DebugPrint("create pixel buffers");
}

void GraphicsCore::DestroyPixelBuffer() {
	GraphicsCore::sCommandListManager.IdleGPU();
	for (auto& colorBuffer : sFrameBuffers) {
		colorBuffer.reset();
	}
	sDepthBuffer.reset();
	sShadowMaskBuffer.Destroy();
	Log::DebugPrint("destroy pixel buffers");
}

void GraphicsCore::FullScreenDraw(GraphicsContext& context) {
	auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("CopyImage");

	context.SetPipelineState(defaultPSO);
	context.SetRootSignature(*defaultRootSignature.get());
	context.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context.SetDynamicDescriptor(rootIndex["gTexture"], 0, sPostEffectBuffer.GetSRV());
	context.Draw(3);
}

void GraphicsCore::InitPostEffect() {
	ShaderModule defaultVS(ShaderStage::Vertex, L"resources/engine/Shaders/FullScreen.VS.hlsl", L"vs_6_0");
	ShaderModule defaultPS(ShaderStage::Pixel, L"resources/engine/Shaders/CopyImage.PS.hlsl", L"ps_6_0");

	const ShaderReflection& vsReflection = defaultVS.GetReflection();
	const ShaderReflection& psReflection = defaultPS.GetReflection();
	std::vector<ShaderReflection> refls;
	refls.push_back(vsReflection);
	refls.push_back(psReflection);

	 defaultRootSignature = std::make_unique<RootSignature>();
	RootSignatureBuilder::BuildFromReflection(refls, *defaultRootSignature, ConvertString(L"CopyImage"));

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = false;

	defaultPSO.SetRootSignature(*defaultRootSignature);
	defaultPSO.SetRasterizerState(rasterizerDesc);
	defaultPSO.SetBlendState(blendDesc);
	defaultPSO.SetDepthStencilState(depthStencilDesc);
	defaultPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	DXGI_FORMAT rtvFormat[] = { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };
	defaultPSO.SetRenderTargetFormats(1, rtvFormat, DXGI_FORMAT_UNKNOWN);
	defaultPSO.SetVertexShader(defaultVS.GetBytecode());
	defaultPSO.SetPixelShader(defaultPS.GetBytecode());
	defaultPSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
	defaultPSO.Finalize();
	

}

void GraphicsCore::CheckRaytracingEnable() {
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 option = {};
	HRESULT hr = sGraphicsDevice->GetDevice()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &option, sizeof(option));

	if (FAILED(hr) || option.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED) {
		sIsEnableRaytracing =  false; // Raytracingがサポートされていない
	}

	sIsEnableRaytracing = true; // Raytracingがサポートされていたら有効にする。
}

}