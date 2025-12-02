#include "GraphicsSwapChain.h"

#include "../GraphicsCore.h"
#include "../Command/CommandListManager.h"

#include "../../Debug/Logger/Log.h"

namespace NoEngine {
namespace Graphics {

namespace {
DXGI_FORMAT SwapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
}

GraphicsSwapChain::GraphicsSwapChain(HWND hwnd, float windowWidth, float windowHeight, uint32_t bufferCount) :
	isResize_(false) {
	Initialize(hwnd, static_cast<UINT>(windowWidth), static_cast<UINT>(windowHeight), static_cast<UINT>(bufferCount));
}

GraphicsSwapChain::GraphicsSwapChain(HWND hwnd, uint32_t windowWidth, uint32_t windowHeight, uint32_t bufferCount) :
	isResize_(false) {
	Initialize(hwnd, static_cast<UINT>(windowWidth), static_cast<UINT>(windowHeight), static_cast<UINT>(bufferCount));
}

GraphicsSwapChain::~GraphicsSwapChain() {
	Destroy();
}


void GraphicsSwapChain::ResizeSignal(UINT newWidth, UINT newHeight) {
	isResize_ = true;
	newWidth_ = newWidth;
	newHeight_ = newHeight;
}


void GraphicsSwapChain::Resize() {
	if (!isResize_) {
		return;
	}
	HRESULT hr = swapChain_->ResizeBuffers(
		2,
		newWidth_,
		newHeight_,
		DXGI_FORMAT_UNKNOWN,
		0
	);
	if (FAILED(hr)) {
		Log::DebugPrint("SwapChain resize failed", VerbosityLevel::kCritical);
		assert(false);
	}
	isResize_ = false;
}

void GraphicsSwapChain::Destroy() {
	swapChain_.Reset();
}

void GraphicsSwapChain::Initialize(HWND hwnd, UINT windowWidth, UINT windowHeight, UINT bufferCount) {
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = windowWidth;
	swapChainDesc.Height = windowHeight;
	swapChainDesc.Format = SwapChainFormat;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = bufferCount;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swapChainDesc.Flags = 0;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
	fsSwapChainDesc.Windowed = TRUE;


	HRESULT hr = GraphicsCore::gGraphicsInfrastructures->GetDXGIFactory()->
		CreateSwapChainForHwnd(
			GraphicsCore::gCommandListManager.GetCommandQueue(),
			hwnd,
			&swapChainDesc,
			&fsSwapChainDesc,
			nullptr,
			reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
	if (FAILED(hr)) {
		Log::DebugPrint("SwapChain.create.failed", VerbosityLevel::kCritical);
		assert(false);
	}
}

}
}